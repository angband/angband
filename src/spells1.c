/*
 * File: spells1.c
 * Purpose: Some spell effects, and the project() function
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
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
#include "cave.h"
#include "dungeon.h"
#include "generate.h"
#include "grafmode.h"
#include "monster/mon-make.h"
#include "monster/mon-msg.h"
#include "monster/mon-spell.h"
#include "monster/mon-timed.h"
#include "monster/mon-util.h"
#include "object/object.h"
#include "object/tvalsval.h"
#include "spells.h"
#include "squelch.h"
#include "trap.h"

#pragma mark floor handlers

typedef struct project_floor_handler_context_s {
	const int who;
	const int r;
	const int y;
	const int x;
	const int dam;
	const int type;
	bool obvious;
} project_floor_handler_context_t;
typedef void (*project_floor_handler_f)(project_floor_handler_context_t *);

/* Destroy Traps (and Locks) */
static void project_floor_handler_KILL_TRAP(project_floor_handler_context_t *context)
{
	const int x = context->x;
	const int y = context->y;

	/* Reveal secret doors */
	if (square_issecretdoor(cave, y, x))
	{
		place_closed_door(cave, y, x);

		/* Check line of sight */
		if (player_has_los_bold(y, x))
		{
			context->obvious = TRUE;
		}
	}

	/* Destroy traps */
	if (square_istrap(cave, y, x))
	{
		/* Check line of sight */
		if (player_has_los_bold(y, x))
		{
			msg("There is a bright flash of light!");
			context->obvious = TRUE;
		}

		/* Forget the trap */
		sqinfo_off(cave->info[y][x], SQUARE_MARK);

		/* Destroy the trap */
		square_destroy_trap(cave, y, x);
	}

	/* Locked doors are unlocked */
	else if (square_islockeddoor(cave, y, x))
	{
		/* Unlock the door */
		square_unlock_door(cave, y, x);

		/* Check line of sound */
		if (player_has_los_bold(y, x))
		{
			msg("Click!");
			context->obvious = TRUE;
		}
	}
}

/* Destroy Doors (and traps) */
static void project_floor_handler_KILL_DOOR(project_floor_handler_context_t *context)
{
	const int x = context->x;
	const int y = context->y;

	/* Destroy all doors and traps */
	if (square_istrap(cave, y, x) || square_isdoor(cave, y, x))
	{
		/* Check line of sight */
		if (player_has_los_bold(y, x))
		{
			/* Message */
			msg("There is a bright flash of light!");
			context->obvious = TRUE;

			/* Visibility change */
			if (square_isdoor(cave, y, x))
			{
				/* Update the visuals */
				p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
			}
		}

		/* Forget the door */
		sqinfo_off(cave->info[y][x], SQUARE_MARK);

		/* Destroy the feature */
		if (square_isdoor(cave, y, x))
			square_destroy_door(cave, y, x);
		else
			square_destroy_trap(cave, y, x);
	}
}

/* Destroy walls (and doors) */
static void project_floor_handler_KILL_WALL(project_floor_handler_context_t *context)
{
	const int x = context->x;
	const int y = context->y;

	/* Non-walls (etc) */
	if (square_ispassable(cave, y, x)) return;

	/* Permanent walls */
	if (square_isperm(cave, y, x)) return;

	/* Granite */
	if (square_iswall(cave, y, x) && !square_hasgoldvein(cave, y, x))
	{
		/* Message */
		if (sqinfo_on(cave->info[y][x], SQUARE_MARK))
		{
			msg("The wall turns into mud!");
			context->obvious = TRUE;
		}

		/* Forget the wall */
		sqinfo_off(cave->info[y][x], SQUARE_MARK);

		/* Destroy the wall */
		square_destroy_wall(cave, y, x);
	}

	/* Quartz / Magma with treasure */
	else if (square_iswall(cave, y, x) && square_hasgoldvein(cave, y, x))
	{
		/* Message */
		if (sqinfo_on(cave->info[y][x], SQUARE_MARK))
		{
			msg("The vein turns into mud!");
			msg("You have found something!");
			context->obvious = TRUE;
		}

		/* Forget the wall */
		sqinfo_off(cave->info[y][x], SQUARE_MARK);

		/* Destroy the wall */
		square_destroy_wall(cave, y, x);

		/* Place some gold */
		place_gold(cave, y, x, p_ptr->depth, ORIGIN_FLOOR);
	}

	/* Quartz / Magma */
	else if (square_ismagma(cave, y, x) || square_isquartz(cave, y, x))
	{
		/* Message */
		if (sqinfo_on(cave->info[y][x], SQUARE_MARK))
		{
			msg("The vein turns into mud!");
			context->obvious = TRUE;
		}

		/* Forget the wall */
		sqinfo_off(cave->info[y][x], SQUARE_MARK);

		/* Destroy the wall */
		square_destroy_wall(cave, y, x);
	}

	/* Rubble */
	else if (square_isrubble(cave, y, x))
	{
		/* Message */
		if (sqinfo_on(cave->info[y][x], SQUARE_MARK))
		{
			msg("The rubble turns into mud!");
			context->obvious = TRUE;
		}

		/* Forget the wall */
		sqinfo_off(cave->info[y][x], SQUARE_MARK);

		/* Destroy the rubble */
		square_destroy_rubble(cave, y, x);

		/* Hack -- place an object */
		if (randint0(100) < 10){
			if (player_can_see_bold(y, x)) {
				msg("There was something buried in the rubble!");
				context->obvious = TRUE;
			}
			place_object(cave, y, x, p_ptr->depth, FALSE, FALSE,
						 ORIGIN_RUBBLE, 0);
		}
	}

	/* Destroy doors (and secret doors) */
	else if (square_isdoor(cave, y, x))
	{
		/* Hack -- special message */
		if (sqinfo_on(cave->info[y][x], SQUARE_MARK))
		{
			msg("The door turns into mud!");
			context->obvious = TRUE;
		}

		/* Forget the wall */
		sqinfo_off(cave->info[y][x], SQUARE_MARK);

		/* Destroy the feature */
		square_destroy_door(cave, y, x);
	}

	/* Update the visuals */
	p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Fully update the flow */
	p_ptr->update |= (PU_FORGET_FLOW | PU_UPDATE_FLOW);
}

/* Make doors */
static void project_floor_handler_MAKE_DOOR(project_floor_handler_context_t *context)
{
	const int x = context->x;
	const int y = context->y;

	/* Require a grid without monsters */
	if (cave->m_idx[y][x]) return;

	/* Require a floor grid */
	if (!square_isfloor(cave, y, x)) return;

	/* Push objects off the grid */
	if (cave->o_idx[y][x]) push_object(y,x);

	/* Create closed door */
	square_add_door(cave, y, x, TRUE);

	/* Observe */
	if (sqinfo_on(cave->info[y][x], SQUARE_MARK)) context->obvious = TRUE;

	/* Update the visuals */
	p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
}

/* Make traps */
static void project_floor_handler_MAKE_TRAP(project_floor_handler_context_t *context)
{
	const int x = context->x;
	const int y = context->y;

	/* Require an "empty", non-warded floor grid */
	if (!square_isempty(cave, y, x)) return;
	if (square_iswarded(cave, y, x)) return;

	/* Create a trap */
	place_trap(cave, y, x, -1, cave->depth);
}

/* Light up the grid */
static void project_floor_handler_LIGHT(project_floor_handler_context_t *context)
{
	const int x = context->x;
	const int y = context->y;

	/* Turn on the light */
	sqinfo_on(cave->info[y][x], SQUARE_GLOW);

	/* Grid is in line of sight */
	if (player_has_los_bold(y, x))
	{
		if (!p_ptr->timed[TMD_BLIND])
		{
			/* Observe */
			context->obvious = TRUE;
		}

		/* Fully update the visuals */
		p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);
	}
}

/* Darken the grid */
static void project_floor_handler_DARK(project_floor_handler_context_t *context)
{
	const int x = context->x;
	const int y = context->y;

	if (p_ptr->depth != 0 || !is_daytime())
	{
		/* Turn off the light */
		sqinfo_off(cave->info[y][x], SQUARE_GLOW);

		/* Hack -- Forget "boring" grids */
		if (!square_isinteresting(cave, y, x))
			sqinfo_off(cave->info[y][x], SQUARE_MARK);
	}

	/* Grid is in line of sight */
	if (player_has_los_bold(y, x))
	{
		/* Observe */
		context->obvious = TRUE;

		/* Fully update the visuals */
		p_ptr->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);
	}
}

#pragma mark object handlers

typedef struct project_object_handler_context_s {
	const int who;
	const int r;
	const int y;
	const int x;
	const int dam;
	const int type;
	bitflag const * const flags;
	object_type *o_ptr; /* Ideally, this would be const, but we can't with C89 initialization. */
	bool obvious;
	bool do_kill;
	bool ignore;
	const char *note_kill;
} project_object_handler_context_t;
typedef void (*project_object_handler_f)(project_object_handler_context_t *);

/**
 * Project an effect onto an object.
 *
 * \param context is the project_o context.
 * \param hate_flag is the OF_ flag for elements that will destroy and object.
 * \param ignore_flag is the OF_flag for elements that the object is immunte to.
 * \param singular_verb is the verb that is displayed when one object is destroyed.
 * \param plural_verb is the verb that is displayed in multiple objects are destroyed.
 */
static void project_object_elemental(project_object_handler_context_t *context, int hate_flag, int ignore_flag, const char *singular_verb, const char *plural_verb)
{
	if (of_has(context->flags, hate_flag)) {
		context->do_kill = TRUE;
		context->note_kill = VERB_AGREEMENT(context->o_ptr->number, singular_verb, plural_verb);
		context->ignore = of_has(context->flags, ignore_flag);
	}
}

/* Acid -- Lots of things */
static void project_object_handler_ACID(project_object_handler_context_t *context)
{
	project_object_elemental(context, OF_HATES_ACID, OF_IGNORE_ACID, "melts", "melt");
}

/* Elec -- Rings and Wands */
static void project_object_handler_ELEC(project_object_handler_context_t *context)
{
	project_object_elemental(context, OF_HATES_ELEC, OF_IGNORE_ELEC, "is destroyed", "are destroyed");
}

/* Fire -- Flammable objects */
static void project_object_handler_FIRE(project_object_handler_context_t *context)
{
	project_object_elemental(context, OF_HATES_FIRE, OF_IGNORE_FIRE, "burns up", "burn up");
}

/* Cold -- potions and flasks */
static void project_object_handler_COLD(project_object_handler_context_t *context)
{
	project_object_elemental(context, OF_HATES_COLD, OF_IGNORE_COLD, "shatters", "shatter");
}

/* Fire + Elec */
static void project_object_handler_PLASMA(project_object_handler_context_t *context)
{
	project_object_elemental(context, OF_HATES_FIRE, OF_IGNORE_FIRE, "burns up", "burn up");
	project_object_elemental(context, OF_HATES_ELEC, OF_IGNORE_ELEC, "is destroyed", "are destroyed");
}

/* Fire + Cold */
static void project_object_handler_METEOR(project_object_handler_context_t *context)
{
	project_object_elemental(context, OF_HATES_FIRE, OF_IGNORE_FIRE, "burns up", "burn up");
	project_object_elemental(context, OF_HATES_COLD, OF_IGNORE_COLD, "shatters", "shatter");
}

/* Hack -- break potions and such */
static void project_object_handler_shatter(project_object_handler_context_t *context)
{
	/* We don't care if the object ignores anything. */
	project_object_elemental(context, OF_HATES_COLD, OF_NONE, "shatters", "shatter");
}

/* Mana -- destroys everything */
static void project_object_handler_MANA(project_object_handler_context_t *context)
{
	context->do_kill = TRUE;
	context->note_kill = VERB_AGREEMENT(context->o_ptr->number, "is destroyed", "are destroyed");
}

/* Holy Orb -- destroys cursed non-artifacts */
static void project_object_handler_HOLY_ORB(project_object_handler_context_t *context)
{
	if (cursed_p(context->o_ptr->flags)) {
		context->do_kill = TRUE;
		context->note_kill = VERB_AGREEMENT(context->o_ptr->number, "is destroyed", "are destroyed");
	}
}

/* Unlock chests */
static void project_object_handler_chest(project_object_handler_context_t *context)
{
	/* Chests are noticed only if trapped or locked */
	if (is_locked_chest(context->o_ptr)) {
		/* Disarm or Unlock */
		unlock_chest((object_type * const)context->o_ptr);

		/* Identify */
		object_notice_everything((object_type * const)context->o_ptr);

		/* Notice */
		if (context->o_ptr->marked > MARK_UNAWARE && !squelch_item_ok(context->o_ptr)) {
			msg("Click!");
			context->obvious = TRUE;
		}
	}
}

#pragma mark monster handlers

typedef struct project_monster_handler_context_s {
	const int who;
	const int r;
	const int y;
	const int x;
	int dam;
	const int type;
	bool seen; /* Ideally, this would be const, but we can't with C89 initialization. */
	const bool id;
	monster_type *m_ptr;
	monster_lore *l_ptr;
	bool obvious;
	bool skipped;
	u16b flag;
	bool do_poly;
	int teleport_distance;
	enum mon_messages hurt_msg;
	enum mon_messages die_msg;
	int mon_timed[MON_TMD_MAX];
} project_monster_handler_context_t;
typedef void (*project_monster_handler_f)(project_monster_handler_context_t *);

/**
 * Resist an attack if the monster has the given elemental flag.
 *
 * If the effect is seen, we learn that the monster has a given flag. Resistance is divided
 * by the factor.
 *
 * \param context is the project_m context.
 * \param flag is the RF_ flag that the monster must have.
 * \param factor is the divisor for the base damage.
 */
static void project_monster_resist_element(project_monster_handler_context_t *context, int flag, int factor)
{
	if (context->seen) rf_on(context->l_ptr->flags, flag);
	if (rf_has(context->m_ptr->race->flags, flag)) {
		context->hurt_msg = MON_MSG_RESIST_A_LOT;
		context->dam /= factor;
	}
}

/**
 * Resist an attack if the monster has the given flag.
 *
 * If the effect is seen, we learn that the monster has a given flag. Resistance is multiplied
 * by the factor and reduced by a small random amount (if reduce is set).
 *
 * \param context is the project_m context.
 * \param flag is the RF_ flag that the monster must have.
 * \param factor is the multiplier for the base damage.
 * \param reduce should be TRUE if the base damage * factor should be reduced, FALSE if the base damage should be increased.
 * \param msg is the message that should be displayed when the monster is hurt.
 */
static void project_monster_resist_other(project_monster_handler_context_t *context, int flag, int factor, bool reduce, enum mon_messages msg)
{
	if (context->seen) rf_on(context->l_ptr->flags, flag);
	if (rf_has(context->m_ptr->race->flags, flag)) {
		context->hurt_msg = msg;
		context->dam *= factor;

		if (reduce)
			context->dam /= randint1(6) + 6;
	}
}

/**
 * Resist an attack if the monster has the given flag or hurt the monster more if it has another flag.
 *
 * If the effect is seen, we learn the status of both flags. Resistance is divided by imm_factor while
 * hurt is multiplied by hurt_factor.
 *
 * \param context is the project_m context.
 * \param hurt_flag is the RF_ flag that the monster must have to use the hurt factor.
 * \param imm_flag is the RF_ flag that the monster must have to use the resistance factor.
 * \param hurt_factor is the hurt multiplier for the base damage.
 * \param imm_factor is the resistance divisor for the base damage.
 * \param hurt_msg is the message that should be displayed when the monster is hurt.
 * \param die_msg is the message that should be displayed when the monster dies.
 */
static void project_monster_hurt_immune(project_monster_handler_context_t *context, int hurt_flag, int imm_flag, int hurt_factor, int imm_factor, enum mon_messages hurt_msg, enum mon_messages die_msg)
{
	if (context->seen) {
		rf_on(context->l_ptr->flags, imm_flag);
		rf_on(context->l_ptr->flags, hurt_flag);
	}

	if (rf_has(context->m_ptr->race->flags, imm_flag)) {
		context->hurt_msg = MON_MSG_RESIST_A_LOT;
		context->dam /= imm_factor;
	}
	else if (rf_has(context->m_ptr->race->flags, hurt_flag)) {
		context->hurt_msg = hurt_msg;
		context->die_msg = die_msg;
		context->dam *= hurt_factor;
	}
}

/**
 * Hurt the monster if it has a given flag or do no damage.
 *
 * If the effect is seen, we learn the status the flag. There is no damage multiplier.
 *
 * \param context is the project_m context.
 * \param flag is the RF_ flag that the monster must have.
 * \param hurt_msg is the message that should be displayed when the monster is hurt.
 * \param die_msg is the message that should be displayed when the monster dies.
 */
static void project_monster_hurt_only(project_monster_handler_context_t *context, int flag, enum mon_messages hurt_msg, enum mon_messages die_msg)
{
	if (context->seen) rf_on(context->l_ptr->flags, flag);

	if (rf_has(context->m_ptr->race->flags, flag)) {
		context->hurt_msg = hurt_msg;
		context->die_msg = die_msg;
	}
	else {
		context->dam = 0;
	}
}

/**
 * Resist an attack if the monster has the given spell flag.
 *
 * If the effect is seen, we learn that the monster has that spell (useful for breaths). Resistance is
 * multiplied by the factor and reduced by a small random amount.
 *
 * \param context is the project_m context.
 * \param flag is the RSF_ flag that the monster must have.
 * \param factor is the multiplier for the base damage.
 */
static void project_monster_breath(project_monster_handler_context_t *context, int flag, int factor)
{
	if (rsf_has(context->m_ptr->race->spell_flags, flag)) {
		/* Learn about breathers through resistance */
		if (context->seen) rsf_on(context->l_ptr->spell_flags, flag);

		context->hurt_msg = MON_MSG_RESIST;
		context->dam *= factor;
		context->dam /= randint1(6) + 6;
	}
}

/**
 * Add a timed status effect to a monster with damage.
 *
 * The source of the damage is tracked if comes from another monster.
 *
 * \param context is the project_m context.
 * \param type is the MON_TMD timer to increment.
 * \param player_amount is the amount to increment the timer by if the source is the player.
 * \param monster_amount is the amount to increment the timer by if the source is another monster.
 */
static void project_monster_timed_damage(project_monster_handler_context_t *context, int type, int player_amount, int monster_amount)
{
	if (type < 0 || type >= MON_TMD_MAX)
		return;

	if (context->who > 0) {
		context->mon_timed[type] = monster_amount;
		context->flag |= MON_TMD_MON_SOURCE;
	}
	else
		context->mon_timed[type] = player_amount;
}

/**
 * Add a timed status effect to a monster without damage.
 *
 * \param context is the project_m context.
 * \param type is the MON_TMD timer to increment.
 */
static void project_monster_timed_no_damage(project_monster_handler_context_t *context, int type)
{
	project_monster_timed_damage(context, type, 0, 0);
	context->dam = 0;
}

/**
 * Teleport away a monster that has a given flag.
 *
 * If the monster matches, it is teleported and the effect is obvious (if seen). The player learns
 * monster lore on whether or not the monster matches the given flag if the effect is seen. Damage
 * is not incurred by the monster.
 *
 * \param context is the project_m context.
 * \param flag is the RF_ flag that the monster must have.
 */
static void project_monster_teleport_away(project_monster_handler_context_t *context, int flag)
{
	if (context->seen) rf_on(context->l_ptr->flags, flag);

	if (rf_has(context->m_ptr->race->flags, flag)) {
		if (context->seen) context->obvious = TRUE;
		context->teleport_distance = context->dam;
		context->hurt_msg = MON_MSG_DISAPPEAR;
	}
	else {
		context->skipped = TRUE;
	}

	context->dam = 0;
}

/**
 * Scare a monster that has a given flag.
 *
 * If the monster matches, fear is applied and the effect is obvious (if seen). The player learns
 * monster lore on whether or not the monster matches the given flag if the effect is seen. Damage
 * is not incurred by the monster.
 *
 * \param context is the project_m context.
 * \param flag is the RF_ flag that the monster must have.
 */
static void project_monster_scare(project_monster_handler_context_t *context, int flag)
{
    if (context->seen) rf_on(context->l_ptr->flags, flag);

	if (rf_has(context->m_ptr->race->flags, flag)) {
		if (context->seen) context->obvious = TRUE;
        project_monster_timed_no_damage(context, MON_TMD_FEAR);
	}
	else {
		context->skipped = TRUE;
	}

	context->dam = 0;
}

/**
 * Dispel a monster that has a given flag.
 *
 * If the monster matches, damage is applied and the effect is obvious (if seen). Otherwise, no damage
 * is applied and the effect is not obvious. The player learns monster lore on whether or not the
 * monster matches the given flag if the effect is seen.
 *
 * \param context is the project_m context.
 * \param flag is the RF_ flag that the monster must have.
 */
static void project_monster_dispel(project_monster_handler_context_t *context, int flag)
{
	if (context->seen) rf_on(context->l_ptr->flags, flag);

	if (rf_has(context->m_ptr->race->flags, flag)) {
		if (context->seen) context->obvious = TRUE;
		context->hurt_msg = MON_MSG_SHUDDER;
		context->die_msg = MON_MSG_DISSOLVE;
	}
	else {
		context->skipped = TRUE;
		context->dam = 0;
	}
}

/* Acid */
static void project_monster_handler_ACID(project_monster_handler_context_t *context)
{
	project_monster_resist_element(context, RF_IM_ACID, 9);
}

/* Electricity */
static void project_monster_handler_ELEC(project_monster_handler_context_t *context)
{
	project_monster_resist_element(context, RF_IM_ELEC, 9);
}

/* Fire damage */
static void project_monster_handler_FIRE(project_monster_handler_context_t *context)
{
	project_monster_hurt_immune(context, RF_HURT_FIRE, RF_IM_FIRE, 2, 9, MON_MSG_CATCH_FIRE, MON_MSG_DISENTEGRATES);
}

/* Cold */
static void project_monster_handler_COLD(project_monster_handler_context_t *context)
{
	project_monster_hurt_immune(context, RF_HURT_COLD, RF_IM_COLD, 2, 9, MON_MSG_BADLY_FROZEN, MON_MSG_FREEZE_SHATTER);
}

/* Ice -- Cold + Stun */
static void project_monster_handler_ICE(project_monster_handler_context_t *context)
{
	int player_amount = (randint1(15) + context->r + p_ptr->lev / 5) / (context->r + 1);
	int monster_amount = (randint1(15) + context->r) / (context->r + 1);
	project_monster_timed_damage(context, MON_TMD_STUN, player_amount, monster_amount);
	project_monster_hurt_immune(context, RF_HURT_COLD, RF_IM_COLD, 2, 9, MON_MSG_BADLY_FROZEN, MON_MSG_FREEZE_SHATTER);
}

/* Poison */
static void project_monster_handler_POIS(project_monster_handler_context_t *context)
{
	project_monster_resist_element(context, RF_IM_POIS, 9);
}

/* Holy Orb -- hurts Evil */
static void project_monster_handler_HOLY_ORB(project_monster_handler_context_t *context)
{
	project_monster_resist_other(context, RF_EVIL, 2, FALSE, MON_MSG_HIT_HARD);
}

/* Plasma */
static void project_monster_handler_PLASMA(project_monster_handler_context_t *context)
{
	project_monster_resist_other(context, RF_RES_PLAS, 3, TRUE, MON_MSG_RESIST);
}

/* Nether -- see above */
static void project_monster_handler_NETHER(project_monster_handler_context_t *context)
{
	/* Update the lore */
	if (context->seen) {
		/* Acquire knowledge of undead type and nether resistance */
		rf_on(context->l_ptr->flags, RF_UNDEAD);
		rf_on(context->l_ptr->flags, RF_RES_NETH);

		/* If it isn't undead, acquire extra knowledge */
		if (!rf_has(context->m_ptr->race->flags, RF_UNDEAD)) {
			/* Learn this creature breathes nether if true */
			if (rsf_has(context->m_ptr->race->spell_flags, RSF_BR_NETH)) {
				rsf_on(context->l_ptr->spell_flags, RSF_BR_NETH);
			}

			/* Otherwise learn about evil type */
			else {
				rf_on(context->l_ptr->flags, RF_EVIL);
			}
		}
	}

	if (rf_has(context->m_ptr->race->flags, RF_UNDEAD)) {
		context->hurt_msg = MON_MSG_IMMUNE;
		context->dam = 0;
	}
	else if (rf_has(context->m_ptr->race->flags, RF_RES_NETH)) {
		context->hurt_msg = MON_MSG_RESIST;
		context->dam *= 3;
		context->dam /= (randint1(6)+6);
	}
	else if (rf_has(context->m_ptr->race->flags, RF_EVIL)) {
		context->dam /= 2;
		context->hurt_msg = MON_MSG_RESIST_SOMEWHAT;
	}
}

/* Water damage */
static void project_monster_handler_WATER(project_monster_handler_context_t *context)
{
	/* Zero out the damage because this is an immunity flag. */
	project_monster_resist_other(context, RF_IM_WATER, 0, FALSE, MON_MSG_IMMUNE);
}

/* Chaos -- Chaos breathers resist */
static void project_monster_handler_CHAOS(project_monster_handler_context_t *context)
{
	int player_amount = (5 + randint1(11) + context->r + p_ptr->lev / 5) / (context->r + 1);
	int monster_amount = (5 + randint1(11) + context->r) / (context->r + 1);

	/* Prevent polymorph on chaos breathers. */
	if (rsf_has(context->m_ptr->race->spell_flags, RSF_BR_CHAO))
		context->do_poly = FALSE;
	else
		context->do_poly = TRUE;

	/* Hide resistance message (as assigned in project_monster_breath()). */
	project_monster_timed_damage(context, MON_TMD_CONF, player_amount, monster_amount);
	project_monster_breath(context, RSF_BR_CHAO, 3);
	context->hurt_msg = MON_MSG_NONE;
}

/* Shards -- Shard breathers resist */
static void project_monster_handler_SHARD(project_monster_handler_context_t *context)
{
	project_monster_breath(context, RSF_BR_SHAR, 3);
}

/* Sound -- Sound breathers resist */
static void project_monster_handler_SOUND(project_monster_handler_context_t *context)
{
	int player_amount = (10 + randint1(15) + context->r + p_ptr->lev / 5) / (context->r + 1);
	int monster_amount = (10 + randint1(15) + context->r) / (context->r + 1);
	project_monster_timed_damage(context, MON_TMD_STUN, player_amount, monster_amount);
	project_monster_breath(context, RSF_BR_SOUN, 2);
}

/* Disenchantment */
static void project_monster_handler_DISEN(project_monster_handler_context_t *context)
{
	project_monster_resist_other(context, RF_RES_DISE, 3, TRUE, MON_MSG_RESIST);
}

/* Nexus */
static void project_monster_handler_NEXUS(project_monster_handler_context_t *context)
{
	project_monster_resist_other(context, RF_RES_NEXUS, 3, TRUE, MON_MSG_RESIST);
}

/* Force */
static void project_monster_handler_FORCE(project_monster_handler_context_t *context)
{
	int player_amount = (randint1(15) + context->r + p_ptr->lev / 5) / (context->r + 1);
	int monster_amount = (randint1(15) + context->r) / (context->r + 1);
	project_monster_timed_damage(context, MON_TMD_STUN, player_amount, monster_amount);
	project_monster_breath(context, RSF_BR_WALL, 3);
}

/* Inertia -- breathers resist */
static void project_monster_handler_INERTIA(project_monster_handler_context_t *context)
{
	project_monster_breath(context, RSF_BR_INER, 3);
}

/* Time -- breathers resist */
static void project_monster_handler_TIME(project_monster_handler_context_t *context)
{
	project_monster_breath(context, RSF_BR_TIME, 3);
}

/* Gravity -- breathers resist */
static void project_monster_handler_GRAVITY(project_monster_handler_context_t *context)
{
	/* Higher level monsters can resist the teleportation better */
	if (randint1(127) > context->m_ptr->race->level)
		context->teleport_distance = 10;

	/* Prevent displacement on gravity breathers. */
	if (rsf_has(context->m_ptr->race->spell_flags, RSF_BR_GRAV))
		context->teleport_distance = 0;

	project_monster_breath(context, RSF_BR_GRAV, 3);
}

/* Drain Life */
static void project_monster_handler_OLD_DRAIN(project_monster_handler_context_t *context)
{
	if (context->seen) {
		rf_on(context->l_ptr->flags, RF_UNDEAD);
		rf_on(context->l_ptr->flags, RF_DEMON);
	}
	if (monster_is_nonliving(context->m_ptr->race)) {
		context->hurt_msg = MON_MSG_UNAFFECTED;
		context->obvious = FALSE;
		context->dam = 0;
	}
}

/* Polymorph monster (Use "dam" as "power") */
static void project_monster_handler_OLD_POLY(project_monster_handler_context_t *context)
{
	/* Polymorph later */
	context->do_poly = context->dam;

	/* No "real" damage */
	context->dam = 0;
}

/* Clone monsters (Ignore "dam") */
static void project_monster_handler_OLD_CLONE(project_monster_handler_context_t *context)
{
	/* Heal fully */
	context->m_ptr->hp = context->m_ptr->maxhp;

	/* Speed up */
	mon_inc_timed(context->m_ptr, MON_TMD_FAST, 50, MON_TMD_FLG_NOTIFY, context->id);

	/* Attempt to clone. */
	if (multiply_monster(context->m_ptr))
		context->hurt_msg = MON_MSG_SPAWN;

	/* No "real" damage */
	context->dam = 0;

}

/* Heal Monster (use "dam" as amount of healing) */
static void project_monster_handler_OLD_HEAL(project_monster_handler_context_t *context)
{
	/* Wake up */
	mon_clear_timed(context->m_ptr, MON_TMD_SLEEP, MON_TMD_FLG_NOMESSAGE, context->id);

	/* Heal */
	context->m_ptr->hp += context->dam;

	/* No overflow */
	if (context->m_ptr->hp > context->m_ptr->maxhp) context->m_ptr->hp = context->m_ptr->maxhp;

	/* Redraw (later) if needed */
	if (p_ptr->health_who == context->m_ptr) p_ptr->redraw |= (PR_HEALTH);

	/* Message */
	else context->hurt_msg = MON_MSG_HEALTHIER;

	/* No "real" damage */
	context->dam = 0;
}

/* Speed Monster (Ignore "dam") */
static void project_monster_handler_OLD_SPEED(project_monster_handler_context_t *context)
{
	project_monster_timed_no_damage(context, MON_TMD_FAST);
}

/* Slow Monster (Use "dam" as "power") */
static void project_monster_handler_OLD_SLOW(project_monster_handler_context_t *context)
{
	project_monster_timed_no_damage(context, MON_TMD_SLOW);
}

/* Sleep (Use "dam" as "power") */
static void project_monster_handler_OLD_SLEEP(project_monster_handler_context_t *context)
{
	project_monster_timed_no_damage(context, MON_TMD_SLEEP);
}

/* Confusion (Use "dam" as "power") */
static void project_monster_handler_OLD_CONF(project_monster_handler_context_t *context)
{
	project_monster_timed_no_damage(context, MON_TMD_CONF);
}

/* Light, but only hurts susceptible creatures */
static void project_monster_handler_LIGHT_WEAK(project_monster_handler_context_t *context)
{
	project_monster_hurt_only(context, RF_HURT_LIGHT, MON_MSG_CRINGE_LIGHT, MON_MSG_SHRIVEL_LIGHT);
}

/* Light -- opposite of Dark */
static void project_monster_handler_LIGHT(project_monster_handler_context_t *context)
{
	if (context->seen) rf_on(context->l_ptr->flags, RF_HURT_LIGHT);

	if (rsf_has(context->m_ptr->race->spell_flags, RSF_BR_LIGHT)) {
		/* Learn about breathers through resistance */
		if (context->seen) rsf_on(context->l_ptr->spell_flags, RSF_BR_LIGHT);

		context->hurt_msg = MON_MSG_RESIST;
		context->dam *= 2;
		context->dam /= randint1(6) + 6;
	}
	else if (rf_has(context->m_ptr->race->flags, RF_HURT_LIGHT)) {
		context->hurt_msg = MON_MSG_CRINGE_LIGHT;
		context->die_msg = MON_MSG_SHRIVEL_LIGHT;
		context->dam *= 2;
	}
}

/* Dark -- opposite of Light */
static void project_monster_handler_DARK(project_monster_handler_context_t *context)
{
	project_monster_breath(context, RSF_BR_DARK, 2);
}

/* Stone to Mud */
static void project_monster_handler_KILL_WALL(project_monster_handler_context_t *context)
{
	project_monster_hurt_only(context, RF_HURT_ROCK, MON_MSG_LOSE_SKIN, MON_MSG_DISSOLVE);
}

/* Teleport undead (Use "dam" as "power") */
static void project_monster_handler_AWAY_UNDEAD(project_monster_handler_context_t *context)
{
	project_monster_teleport_away(context, RF_UNDEAD);
}

/* Teleport evil (Use "dam" as "power") */
static void project_monster_handler_AWAY_EVIL(project_monster_handler_context_t *context)
{
	project_monster_teleport_away(context, RF_EVIL);
}

/* Teleport monster (Use "dam" as "power") */
static void project_monster_handler_AWAY_ALL(project_monster_handler_context_t *context)
{
	/* Prepare to teleport */
	context->teleport_distance = context->dam;

	/* No "real" damage */
	context->dam = 0;
	context->hurt_msg = MON_MSG_DISAPPEAR;
}

/* Turn undead (Use "dam" as "power") */
static void project_monster_handler_TURN_UNDEAD(project_monster_handler_context_t *context)
{
	project_monster_scare(context, RF_UNDEAD);
}

/* Turn evil (Use "dam" as "power") */
static void project_monster_handler_TURN_EVIL(project_monster_handler_context_t *context)
{
	project_monster_scare(context, RF_EVIL);
}

/* Turn monster (Use "dam" as "power") */
static void project_monster_handler_TURN_ALL(project_monster_handler_context_t *context)
{
	project_monster_timed_no_damage(context, MON_TMD_FEAR);
}

/* Dispel undead */
static void project_monster_handler_DISP_UNDEAD(project_monster_handler_context_t *context)
{
	project_monster_dispel(context, RF_UNDEAD);
}

/* Dispel evil */
static void project_monster_handler_DISP_EVIL(project_monster_handler_context_t *context)
{
	project_monster_dispel(context, RF_EVIL);
}

/* Dispel monster */
static void project_monster_handler_DISP_ALL(project_monster_handler_context_t *context)
{
	context->hurt_msg = MON_MSG_SHUDDER;
	context->die_msg = MON_MSG_DISSOLVE;
}

#pragma mark player handlers

typedef struct project_player_handler_context_s {
	const int who;
	const int r;
	const int y;
	const int x;
	const int dam;
	const int type;
	bool obvious;
} project_player_handler_context_t;
typedef void (*project_player_handler_f)(project_player_handler_context_t *);

static void project_player_handler_GRAVITY(project_player_handler_context_t *context)
{
	msg("Gravity warps around you.");
}

#pragma mark other functions

/**
 * Structure for GF types and their resistances/immunities/vulnerabilities
 */
static const struct gf_type {
	u16b name;			/* numerical index (GF_#) */
	const char *desc;	/* text description (if blind) */
	int resist;			/* object flag for resistance */
	int num;			/* numerator for resistance */
	random_value denom;	/* denominator for resistance */
	bool force_obvious;	/* */
	byte color;			/* */
	int opp;			/* timed flag for temporary resistance ("opposition") */
	int immunity;		/* object flag for total immunity */
	bool side_immune;	/* whether immunity protects from ALL side effects */
	int vuln;			/* object flag for vulnerability */
	int mon_res;		/* monster flag for resistance */
	int mon_vuln;		/* monster flag for vulnerability */
	int obj_hates;		/* object flag for object vulnerability */
	int obj_imm;		/* object flag for object immunity */
	project_floor_handler_f floor_handler;
	project_object_handler_f object_handler;
	project_monster_handler_f monster_handler;
	project_player_handler_f player_handler;
} gf_table[] = {
	#define GF(a, b, c, d, e, obv, col, f, g, h, i, j, k, l, m, fh, oh, mh, ph) { GF_##a, b, c, d, e, obv, col, f, g, h, i, j, k, l, m, fh, oh, mh, ph },
	#define RV(b, x, y, m) {b, x, y, m}
	#define FH(x) project_floor_handler_##x
	#define OH(x) project_object_handler_##x
	#define MH(x) project_monster_handler_##x
	#define PH(x) project_player_handler_##x
	#include "list-gf-types.h"
	#undef GF
	#undef RV
	#undef FH
	#undef OH
	#undef MH
	#undef PH
};

static const char *gf_name_list[] =
{
	#define GF(a, b, c, d, e, obv, col, f, g, h, i, j, k, l, m, fh, oh, mh, ph) #a,
	#include "list-gf-types.h"
	#undef GF
    NULL
};

static project_floor_handler_f gf_floor_handler(int type)
{
	if (type < 0 || type >= GF_MAX)
		return NULL;

	return gf_table[type].floor_handler;
}

static project_object_handler_f gf_object_handler(int type)
{
	if (type < 0 || type >= GF_MAX)
		return NULL;

	return gf_table[type].object_handler;
}

static project_monster_handler_f gf_monster_handler(int type)
{
	if (type < 0 || type >= GF_MAX)
		return NULL;

	return gf_table[type].monster_handler;
}

static project_player_handler_f gf_player_handler(int type)
{
	if (type < 0 || type >= GF_MAX)
		return NULL;

	return gf_table[type].player_handler;
}

static bool gf_force_obvious(int type)
{
	if (type < 0 || type >= GF_MAX)
		return FALSE;

	return gf_table[type].force_obvious;
}

static byte gf_color(int type)
{
	if (type < 0 || type >= GF_MAX)
		return TERM_WHITE;

	return gf_table[type].color;
}


/**
 * Check for resistance to a GF_ attack type. Return codes:
 * -1 = vulnerability
 * 0 = no resistance (or resistance plus vulnerability)
 * 1 = single resistance or opposition (or double resist plus vulnerability)
 * 2 = double resistance (including opposition)
 * 3 = total immunity
 *
 * \param type is the attack type we are trying to resist
 * \param flags is the set of flags we're checking
 * \param real is whether this is a real attack
 */
int check_for_resist(struct player *p, int type, bitflag *flags, bool real)
{
	const struct gf_type *gf_ptr = &gf_table[type];
	int result = 0;

	if (gf_ptr->vuln && of_has(flags, gf_ptr->vuln))
		result--;

	/* If it's not a real attack, we don't check timed status explicitly */
	if (real && gf_ptr->opp && p->timed[gf_ptr->opp])
		result++;

	if (gf_ptr->resist && of_has(flags, gf_ptr->resist))
		result++;

	if (gf_ptr->immunity && of_has(flags, gf_ptr->immunity))
		result = 3;

	/* Notice flags, if it's a real attack */
	if (real && gf_ptr->immunity)
		wieldeds_notice_flag(p, gf_ptr->immunity);
	if (real && gf_ptr->resist)
		wieldeds_notice_flag(p, gf_ptr->resist);
	if (real && gf_ptr->vuln)
		wieldeds_notice_flag(p, gf_ptr->vuln);

	return result;
}

/**
 * Check whether the player is immune to side effects of a GF_ type.
 *
 * \param type is the GF_ type we are checking.
 */
bool check_side_immune(int type)
{
	const struct gf_type *gf_ptr = &gf_table[type];

	if (gf_ptr->immunity) {
		if (gf_ptr->side_immune && check_state(p_ptr, gf_ptr->immunity,
				p_ptr->state.flags))
			return TRUE;
	} else if ((gf_ptr->resist && of_has(p_ptr->state.flags, gf_ptr->resist)) ||
				(gf_ptr->opp && p_ptr->timed[gf_ptr->opp]))
		return TRUE;

	return FALSE;
}

/**
 * Update monster knowledge of player resists.
 *
 * \param m is the monster who is learning
 * \param p is the player being learnt about
 * \param type is the GF_ type to which it's learning about the player's
 *    resistance (or lack of)
 */
void monster_learn_resists(struct monster *m, struct player *p, int type)
{
	const struct gf_type *gf_ptr = &gf_table[type];

	update_smart_learn(m, p, gf_ptr->resist);
	update_smart_learn(m, p, gf_ptr->immunity);
	update_smart_learn(m, p, gf_ptr->vuln);

	return;
}

/**
 * Strip the HATES_ flags out of a flagset for any IGNORE_ flags that are
 * present
 */
void dedup_hates_flags(bitflag *f)
{
	size_t i;

	for (i = 0; i < GF_MAX; i++) {
		const struct gf_type *gf_ptr = &gf_table[i];
		if (gf_ptr->obj_imm && of_has(f, gf_ptr->obj_imm) &&
				gf_ptr->obj_hates && of_has(f, gf_ptr->obj_hates))
			of_off(f, gf_ptr->obj_hates);
	}
}

/*
 * Helper function -- return a "nearby" race for polymorphing
 *
 * Note that this function is one of the more "dangerous" ones...
 */
static monster_race *poly_race(monster_race *race)
{
	int i, minlvl, maxlvl, goal;

	assert(race && race->name);

	/* Uniques never polymorph */
	if (rf_has(race->flags, RF_UNIQUE)) return race;

	/* Allowable range of "levels" for resulting monster */
	goal = (p_ptr->depth + race->level) / 2 + 5;
	minlvl = MIN(race->level - 10, (race->level * 3) / 4);
	maxlvl = MAX(race->level + 10, (race->level * 5) / 4);

	/* Small chance to allow something really strong */
	if (one_in_(100)) maxlvl = 100;

	/* Try to pick a new, non-unique race within our level range */
	for (i = 0; i < 1000; i++) {
		monster_race *new_race = get_mon_num(goal);

		if (!new_race || new_race == race) continue;
		if (rf_has(new_race->flags, RF_UNIQUE)) continue;
		if (new_race->level < minlvl || new_race->level > maxlvl) continue;

		/* Avoid force-depth monsters, since it might cause a crash in project_m() */
		if (rf_has(new_race->flags, RF_FORCE_DEPTH) && p_ptr->depth < new_race->level) continue;

		return new_race;
	}

	/* If we get here, we weren't able to find a new race. */
	return race;
}

/*
 * Teleport a monster, normally up to "dis" grids away.
 *
 * Attempt to move the monster at least "dis/2" grids away.
 *
 * But allow variation to prevent infinite loops.
 */
void teleport_away(struct monster *m_ptr, int dis)
{
	int ny = 0, nx = 0, oy, ox, d, i, min;

	bool look = TRUE;


	/* Paranoia */
	if (!m_ptr->race) return;

	/* Save the old location */
	oy = m_ptr->fy;
	ox = m_ptr->fx;

	/* Minimum distance */
	min = dis / 2;

	/* Look until done */
	while (look)
	{
		/* Verify max distance */
		if (dis > 200) dis = 200;

		/* Try several locations */
		for (i = 0; i < 500; i++)
		{
			/* Pick a (possibly illegal) location */
			while (1)
			{
				ny = rand_spread(oy, dis);
				nx = rand_spread(ox, dis);
				d = distance(oy, ox, ny, nx);
				if ((d >= min) && (d <= dis)) break;
			}

			/* Ignore illegal locations */
			if (!square_in_bounds_fully(cave, ny, nx)) continue;

			/* Require "empty" floor space */
			if (!square_isempty(cave, ny, nx)) continue;

			/* Hack -- no teleport onto glyph of warding */
			if (square_iswarded(cave, ny, nx)) continue;

			/* No teleporting into vaults and such */
			/* if (cave->info[ny][nx] & square_isvault(cave, ny, nx)) continue; */

			/* This grid looks good */
			look = FALSE;

			/* Stop looking */
			break;
		}

		/* Increase the maximum distance */
		dis = dis * 2;

		/* Decrease the minimum distance */
		min = min / 2;
	}

	/* Sound */
	sound(MSG_TPOTHER);

	/* Swap the monsters */
	monster_swap(oy, ox, ny, nx);
}

/*
 * Teleport the player to a location up to "dis" grids away.
 *
 * If no such spaces are readily available, the distance may increase.
 * Try very hard to move the player at least a quarter that distance.
 */
void teleport_player(int dis)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int d, i, min, y, x;

	bool look = TRUE;


	/* Initialize */
	y = py;
	x = px;

	/* Minimum distance */
	min = dis / 2;

	/* Look until done */
	while (look)
	{
		/* Verify max distance */
		if (dis > 200) dis = 200;

		/* Try several locations */
		for (i = 0; i < 500; i++)
		{
			/* Pick a (possibly illegal) location */
			while (1)
			{
				y = rand_spread(py, dis);
				x = rand_spread(px, dis);
				d = distance(py, px, y, x);
				if ((d >= min) && (d <= dis)) break;
			}

			/* Ignore illegal locations */
			if (!square_in_bounds_fully(cave, y, x)) continue;

			/* Require "naked" floor space */
			if (!square_isempty(cave, y, x)) continue;

			/* No teleporting into vaults and such */
			if (square_isvault(cave, y, x)) continue;

			/* This grid looks good */
			look = FALSE;

			/* Stop looking */
			break;
		}

		/* Increase the maximum distance */
		dis = dis * 2;

		/* Decrease the minimum distance */
		min = min / 2;
	}

	/* Sound */
	sound(MSG_TELEPORT);

	/* Move player */
	monster_swap(py, px, y, x);

	/* Handle stuff XXX XXX XXX */
	handle_stuff(p_ptr);
}

/*
 * Teleport player to a grid near the given location
 *
 * This function is slightly obsessive about correctness.
 * This function allows teleporting into vaults (!)
 */
void teleport_player_to(int ny, int nx)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int y, x;

	int dis = 0, ctr = 0;

	/* Initialize */
	y = py;
	x = px;

	/* Find a usable location */
	while (1)
	{
		/* Pick a nearby legal location */
		while (1)
		{
			y = rand_spread(ny, dis);
			x = rand_spread(nx, dis);
			if (square_in_bounds_fully(cave, y, x)) break;
		}

		/* Accept "naked" floor grids */
		if (square_isempty(cave, y, x)) break;

		/* Occasionally advance the distance */
		if (++ctr > (4 * dis * dis + 4 * dis + 1))
		{
			ctr = 0;
			dis++;
		}
	}

	/* Sound */
	sound(MSG_TELEPORT);

	/* Move player */
	monster_swap(py, px, y, x);

	/* Handle stuff XXX XXX XXX */
	handle_stuff(p_ptr);
}

/*
 * Teleport the player one level up or down (random when legal)
 */
void teleport_player_level(void)
{
	bool up = TRUE, down = TRUE;

	/* No going up with force_descend or in the town */
	if (OPT(birth_force_descend) || !p_ptr->depth)
		up = FALSE;

	/* No forcing player down to quest levels if they can't leave */
	if (!up && is_quest(p_ptr->max_depth + 1))
		down = FALSE;

	/* Can't leave quest levels or go down deeper than the dungeon */
	if (is_quest(p_ptr->depth) || (p_ptr->depth >= MAX_DEPTH-1))
		down = FALSE;

	/* Determine up/down if not already done */
	if (up && down) {
		if (randint0(100) < 50)
			up = FALSE;
		else
			down = FALSE;
	}

	/* Now actually do the level change */
	if (up) {
		msgt(MSG_TPLEVEL, "You rise up through the ceiling.");
		dungeon_change_level(p_ptr->depth - 1);
	} else if (down) {
		msgt(MSG_TPLEVEL, "You sink through the floor.");

		if (OPT(birth_force_descend))
			dungeon_change_level(p_ptr->max_depth + 1);
		else
			dungeon_change_level(p_ptr->depth + 1);
	} else {
		msg("Nothing happens.");
	}
}

int gf_name_to_idx(const char *name)
{
    int i;
    for (i = 0; gf_name_list[i]; i++) {
        if (!my_stricmp(name, gf_name_list[i]))
            return i;
    }

    return -1;
}

const char *gf_idx_to_name(int type)
{
    assert(type >= 0);
    assert(type < GF_MAX);

    return gf_name_list[type];
}

/*
 * Return a color to use for the bolt/ball spells
 */
static byte spell_color(int type)
{
	return gf_color(type);
}

/*
 * Find the attr/char pair to use for a spell effect
 *
 * It is moving (or has moved) from (x,y) to (nx,ny).
 *
 * If the distance is not "one", we (may) return "*".
 */
static void bolt_pict(int y, int x, int ny, int nx, int typ, byte *a, wchar_t *c)
{
	int motion;

	/* Convert co-ordinates into motion */
	if ((ny == y) && (nx == x))
		motion = BOLT_NO_MOTION;
	else if (nx == x)
		motion = BOLT_0;
	else if ((ny-y) == (x-nx))
		motion = BOLT_45;
	else if (ny == y)
		motion = BOLT_90;
	else if ((ny-y) == (nx-x))
		motion = BOLT_135;
	else
		motion = BOLT_NO_MOTION;

	/* Decide on output char */
	if (use_graphics == GRAPHICS_NONE) {
		/* ASCII is simple */
		wchar_t chars[] = L"*|/-\\";

		*c = chars[motion];
		*a = spell_color(typ);
	} else {
		*a = gf_to_attr[typ][motion];
		*c = gf_to_char[typ][motion];
	}
}

/*
 * Decreases players hit points and sets death flag if necessary
 *
 * Invulnerability needs to be changed into a "shield" XXX XXX XXX
 *
 * Hack -- this function allows the user to save (or quit) the game
 * when he dies, since the "You die." message is shown before setting
 * the player to "dead".
 */
void take_hit(struct player *p, int dam, const char *kb_str)
{
	int old_chp = p->chp;

	int warning = (p->mhp * op_ptr->hitpoint_warn / 10);


	/* Paranoia */
	if (p->is_dead) return;


	/* Disturb */
	disturb(p, 1, 0);

	/* Mega-Hack -- Apply "invulnerability" */
	if (p->timed[TMD_INVULN] && (dam < 9000)) return;

	/* Hurt the player */
	p->chp -= dam;

	/* Display the hitpoints */
	p->redraw |= (PR_HP);

	/* Dead player */
	if (p->chp < 0)
	{
		/* Hack -- Note death */
		msgt(MSG_DEATH, "You die.");
		message_flush();

		/* Note cause of death */
		my_strcpy(p->died_from, kb_str, sizeof(p->died_from));

		/* No longer a winner */
		p->total_winner = FALSE;

		/* Note death */
		p->is_dead = TRUE;

		/* Leaving */
		p->leaving = TRUE;

		/* Dead */
		return;
	}

	/* Hitpoint warning */
	if (p->chp < warning)
	{
		/* Hack -- bell on first notice */
		if (old_chp > warning)
		{
			bell("Low hitpoint warning!");
		}

		/* Message */
		msgt(MSG_HITPOINT_WARN, "*** LOW HITPOINT WARNING! ***");
		message_flush();
	}
}

/*
 * Destroys a type of item on a given percent chance.
 * The chance 'cperc' is in hundredths of a percent (1-in-10000)
 * Note that missiles are no longer necessarily all destroyed
 *
 * Returns number of items destroyed.
 */
int inven_damage(struct player *p, int type, int cperc)
{
	const struct gf_type *gf_ptr = &gf_table[type];

	int i, j, k, amt;

	object_type *o_ptr;

	char o_name[80];
	
	bool damage;

	bitflag f[OF_SIZE];

	/* Count the casualties */
	k = 0;

	/* Scan through the slots backwards */
	for (i = 0; i < QUIVER_END; i++)
	{
		if (i >= INVEN_PACK && i < QUIVER_START) continue;

		o_ptr = &p->inventory[i];

		of_wipe(f);
		object_flags(o_ptr, f);

		/* Skip non-objects */
		if (!o_ptr->kind) continue;

		/* Hack -- for now, skip artifacts */
		if (o_ptr->artifact) continue;

		/* Give this item slot a shot at death if it is vulnerable */
		if (of_has(f, gf_ptr->obj_hates) &&	!of_has(f, gf_ptr->obj_imm))
		{
			/* Chance to destroy this item */
			int chance = cperc;

			/* Track if it is damaged instead of destroyed */
			damage = FALSE;

			/** 
			 * Analyze the type to see if we just damage it
			 * - we also check for rods to reduce chance
			 */
			switch (o_ptr->tval)
			{
				/* Weapons */
				case TV_BOW:
				case TV_SWORD:
				case TV_HAFTED:
				case TV_POLEARM:
				case TV_DIGGING:
				{
					/* Chance to damage it */
					if (randint0(10000) < cperc)
					{
						/* Damage the item */
						o_ptr->to_h--;
						o_ptr->to_d--;

						/* Damaged! */
						damage = TRUE;
					}
					else continue;

					break;
				}

				/* Wearable items */
				case TV_HELM:
				case TV_CROWN:
				case TV_SHIELD:
				case TV_BOOTS:
				case TV_GLOVES:
				case TV_CLOAK:
				case TV_SOFT_ARMOR:
				case TV_HARD_ARMOR:
				case TV_DRAG_ARMOR:
				{
					/* Chance to damage it */
					if (randint0(10000) < cperc)
					{
						/* Damage the item */
						o_ptr->to_a--;

						/* Damaged! */
						damage = TRUE;
					}
					else continue;

					break;
				}
				
				/* Rods are tough */
				case TV_ROD:
				{
					chance = (chance / 4);
					
					break;
				}
			}

			/* Damage instead of destroy */
			if (damage)
			{
				p->update |= (PU_BONUS);
				p->redraw |= (PR_EQUIP);

				/* Casualty count */
				amt = o_ptr->number;
			}

			/* ... or count the casualties */
			else for (amt = j = 0; j < o_ptr->number; ++j)
			{
				if (randint0(10000) < chance) amt++;
			}

			/* Some casualities */
			if (amt)
			{
				/* Get a description */
				object_desc(o_name, sizeof(o_name), o_ptr,
					ODESC_BASE);

				/* Message */
				msgt(MSG_DESTROY, "%sour %s (%c) %s %s!",
				           ((o_ptr->number > 1) ?
				            ((amt == o_ptr->number) ? "All of y" :
				             (amt > 1 ? "Some of y" : "One of y")) : "Y"),
				           o_name, index_to_label(i),
				           ((amt > 1) ? "were" : "was"),
					   (damage ? "damaged" : "destroyed"));

				/* Damage already done? */
				if (damage) continue;

				/* Reduce charges if some devices are destroyed */
				reduce_charges(o_ptr, amt);

				/* Destroy "amt" items */
				inven_item_increase(i, -amt);
				inven_item_optimize(i);

				/* Count the casualties */
				k += amt;
			}
		}
	}

	/* Return the casualty count */
	return (k);
}

/*
 * Acid has hit the player, attempt to affect some armor.
 *
 * Note that the "base armor" of an object never changes.
 *
 * If any armor is damaged (or resists), the player takes less damage.
 */
static int minus_ac(struct player *p)
{
	object_type *o_ptr = NULL;

	bitflag f[OF_SIZE];

	char o_name[80];

	/* Avoid crash during monster power calculations */
	if (!p->inventory) return FALSE;

	/* Pick a (possibly empty) inventory slot */
	switch (randint1(6))
	{
		case 1: o_ptr = &p->inventory[INVEN_BODY]; break;
		case 2: o_ptr = &p->inventory[INVEN_ARM]; break;
		case 3: o_ptr = &p->inventory[INVEN_OUTER]; break;
		case 4: o_ptr = &p->inventory[INVEN_HANDS]; break;
		case 5: o_ptr = &p->inventory[INVEN_HEAD]; break;
		case 6: o_ptr = &p->inventory[INVEN_FEET]; break;
		default: assert(0);
	}

	/* Nothing to damage */
	if (!o_ptr->kind) return (FALSE);

	/* No damage left to be done */
	if (o_ptr->ac + o_ptr->to_a <= 0) return (FALSE);

	/* Describe */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);

	/* Extract the flags */
	object_flags(o_ptr, f);

	/* Object resists */
	if (of_has(f, OF_IGNORE_ACID))
	{
		msg("Your %s is unaffected!", o_name);

		return (TRUE);
	}

	/* Message */
	msg("Your %s is damaged!", o_name);

	/* Damage the item */
	o_ptr->to_a--;

	p->update |= PU_BONUS;
	p->redraw |= (PR_EQUIP);

	/* Item was damaged */
	return (TRUE);
}

/**
 * Adjust damage according to resistance or vulnerability.
 *
 * \param type is the attack type we are checking.
 * \param dam is the unadjusted damage.
 * \param dam_aspect is the calc we want (min, avg, max, random).
 * \param resist is the degree of resistance (-1 = vuln, 3 = immune).
 */
int adjust_dam(struct player *p, int type, int dam, aspect dam_aspect, int resist)
{
	const struct gf_type *gf_ptr = &gf_table[type];
	int i, denom;

	if (resist == 3) /* immune */
		return 0;

	/* Hack - acid damage is halved by armour, holy orb is halved */
	if ((type == GF_ACID && minus_ac(p)) || type == GF_HOLY_ORB)
		dam = (dam + 1) / 2;

	if (resist == -1) /* vulnerable */
		return (dam * 4 / 3);

	/* Variable resists vary the denominator, so we need to invert the logic
	 * of dam_aspect. (m_bonus is unused) */
	switch (dam_aspect) {
		case MINIMISE:
			denom = randcalc(gf_ptr->denom, 0, MAXIMISE);
			break;
		case MAXIMISE:
			denom = randcalc(gf_ptr->denom, 0, MINIMISE);
			break;
		case AVERAGE:
		case EXTREMIFY:
		case RANDOMISE:
			denom = randcalc(gf_ptr->denom, 0, dam_aspect);
			break;
		default:
			assert(0);
	}

	for (i = resist; i > 0; i--)
		if (denom)
			dam = dam * gf_ptr->num / denom;

	return dam;
}

/*
 * Restore a stat.  Return TRUE only if this actually makes a difference.
 */
bool res_stat(int stat)
{
	/* Restore if needed */
	if (p_ptr->stat_cur[stat] != p_ptr->stat_max[stat])
	{
		/* Restore */
		p_ptr->stat_cur[stat] = p_ptr->stat_max[stat];

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Success */
		return (TRUE);
	}

	/* Nothing to restore */
	return (FALSE);
}

/*
 * Apply disenchantment to the player's stuff
 *
 * This function is also called from the "melee" code.
 *
 * The "mode" is currently unused.
 *
 * Return "TRUE" if the player notices anything.
 */
bool apply_disenchant(int mode)
{
	int t = 0;

	object_type *o_ptr;

	char o_name[80];


	/* Unused parameter */
	(void)mode;

	/* Pick a random slot */
	switch (randint1(8))
	{
		case 1: t = INVEN_WIELD; break;
		case 2: t = INVEN_BOW; break;
		case 3: t = INVEN_BODY; break;
		case 4: t = INVEN_OUTER; break;
		case 5: t = INVEN_ARM; break;
		case 6: t = INVEN_HEAD; break;
		case 7: t = INVEN_HANDS; break;
		case 8: t = INVEN_FEET; break;
	}

	/* Get the item */
	o_ptr = &p_ptr->inventory[t];

	/* No item, nothing happens */
	if (!o_ptr->kind) return (FALSE);


	/* Nothing to disenchant */
	if ((o_ptr->to_h <= 0) && (o_ptr->to_d <= 0) && (o_ptr->to_a <= 0))
	{
		/* Nothing to notice */
		return (FALSE);
	}


	/* Describe the object */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);


	/* Artifacts have 60% chance to resist */
	if (o_ptr->artifact && (randint0(100) < 60))
	{
		/* Message */
		msg("Your %s (%c) resist%s disenchantment!",
		           o_name, index_to_label(t),
		           ((o_ptr->number != 1) ? "" : "s"));

		/* Notice */
		return (TRUE);
	}

	/* Apply disenchantment, depending on which kind of equipment */
	if (t == INVEN_WIELD || t == INVEN_BOW)
	{
		/* Disenchant to-hit */
		if (o_ptr->to_h > 0) o_ptr->to_h--;
		if ((o_ptr->to_h > 5) && (randint0(100) < 20)) o_ptr->to_h--;

		/* Disenchant to-dam */
		if (o_ptr->to_d > 0) o_ptr->to_d--;
		if ((o_ptr->to_d > 5) && (randint0(100) < 20)) o_ptr->to_d--;
	}
	else
	{
		/* Disenchant to-ac */
		if (o_ptr->to_a > 0) o_ptr->to_a--;
		if ((o_ptr->to_a > 5) && (randint0(100) < 20)) o_ptr->to_a--;
	}

	/* Message */
	msg("Your %s (%c) %s disenchanted!",
	           o_name, index_to_label(t),
	           ((o_ptr->number != 1) ? "were" : "was"));

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Window stuff */
	p_ptr->redraw |= (PR_EQUIP);

	/* Notice */
	return (TRUE);
}

/*
 * Mega-Hack -- track "affected" monsters (see "project()" comments)
 */
static int project_m_n;
static int project_m_x;
static int project_m_y;

/*
 * We are called from "project()" to "damage" terrain features
 *
 * We are called both for "beam" effects and "ball" effects.
 *
 * The "r" parameter is the "distance from ground zero".
 *
 * Note that we determine if the player can "see" anything that happens
 * by taking into account: blindness, line-of-sight, and illumination.
 *
 * We return "TRUE" if the effect of the projection is "obvious".
 *
 * Hack -- We also "see" grids which are "memorized".
 *
 * Perhaps we should affect doors and/or walls.
 */
static bool project_f(int who, int r, int y, int x, int dam, int typ, bool obvious)
{
#if 0 /* unused */
	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);
#endif /* 0 */

	project_floor_handler_context_t context = {
		who,
		r,
		y,
		x,
		dam,
		typ,
		obvious,
	};
	project_floor_handler_f floor_handler = gf_floor_handler(typ);

	if (floor_handler != NULL)
		floor_handler(&context);

	/* Return "Anything seen?" */
	return context.obvious;
}

/*
 * We are called from "project()" to "damage" objects
 *
 * We are called both for "beam" effects and "ball" effects.
 *
 * Perhaps we should only SOMETIMES damage things on the ground.
 *
 * The "r" parameter is the "distance from ground zero".
 *
 * Note that we determine if the player can "see" anything that happens
 * by taking into account: blindness, line-of-sight, and illumination.
 *
 * Hack -- We also "see" objects which are "memorized".
 *
 * We return "TRUE" if the effect of the projection is "obvious".
 */
static bool project_o(int who, int r, int y, int x, int dam, int typ, bool obvious)
{
	s16b this_o_idx, next_o_idx = 0;
	bitflag f[OF_SIZE];

#if 0 /* unused */
	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);
#endif /* 0 */

	/* Scan all objects in the grid */
	for (this_o_idx = cave->o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;
		bool ignore = FALSE;
		bool do_kill = FALSE;
		const char *note_kill = NULL;
		project_object_handler_context_t context = {
			who,
			r,
			y,
			x,
			dam,
			typ,
			f,
			NULL,
			obvious,
			do_kill,
			ignore,
			note_kill,
		};
		project_object_handler_f object_handler = gf_object_handler(typ);

		/* Get the object */
		o_ptr = object_byid(this_o_idx);
		context.o_ptr = o_ptr;

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Extract the flags */
		object_flags(o_ptr, f);

		if (object_handler != NULL)
			object_handler(&context);

		obvious = context.obvious;
		do_kill = context.do_kill;
		ignore = context.ignore;
		note_kill = context.note_kill;

		/* Attempt to destroy the object */
		if (do_kill)
		{
			char o_name[80];

			/* Effect "observed" */
			if (o_ptr->marked && !squelch_item_ok(o_ptr))
			{
				obvious = TRUE;
				object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);
			}

			/* Artifacts, and other objects, get to resist */
			if (o_ptr->artifact || ignore)
			{
				/* Observe the resist */
				if (o_ptr->marked && !squelch_item_ok(o_ptr))
					msg("The %s %s unaffected!", o_name, VERB_AGREEMENT(o_ptr->number, "is", "are"));
			}

			/* Reveal mimics */
			else if (o_ptr->mimicking_m_idx) {
				become_aware(cave_monster(cave, o_ptr->mimicking_m_idx));
			}

			/* Kill it */
			else
			{
				/* Describe if needed */
				if (o_ptr->marked && note_kill && !squelch_item_ok(o_ptr))
					msgt(MSG_DESTROY, "The %s %s!", o_name, note_kill);

				/* Delete the object */
				delete_object_idx(this_o_idx);

				/* Redraw */
				square_light_spot(cave, y, x);
			}
		}
	}

	/* Return "Anything seen?" */
	return (obvious);
}

/**
 * Deal damage to a monster from another monster.
 *
 * This is a helper for project_m(). It is very similar to mon_take_hit(), but eliminates the player-oriented
 * stuff of that function. It isn't a type handler, but we take a handler context since that has a lot of
 * what we need.
 *
 * \param context is the project_m context.
 * \param m_name is the formatted monster name.
 * \param m_idx is the cave monster index.
 * \return TRUE if the monster died, FALSE if it is still alive.
 */
static bool project_m_monster_attack(project_monster_handler_context_t *context, const char *m_name, int m_idx)
{
	bool mon_died = FALSE;
	bool seen = context->seen;
	int dam = context->dam;
	enum mon_messages die_msg = context->die_msg;
	enum mon_messages hurt_msg = context->hurt_msg;
	monster_type *m_ptr = context->m_ptr;

	/* "Unique" monsters can only be "killed" by the player */
	if (rf_has(m_ptr->race->flags, RF_UNIQUE)) {
		/* Reduce monster hp to zero, but don't kill it. */
		if (dam > m_ptr->hp) dam = m_ptr->hp;
	}

	/* Redraw (later) if needed */
	if (p_ptr->health_who == m_ptr) p_ptr->redraw |= (PR_HEALTH);

	/* Wake the monster up */
	mon_clear_timed(m_ptr, MON_TMD_SLEEP, MON_TMD_FLG_NOMESSAGE, FALSE);

	/* Hurt the monster */
	m_ptr->hp -= dam;

	/* Dead monster */
	if (m_ptr->hp < 0)
	{
		/* Give detailed messages if destroyed */
		if (!seen) die_msg = MON_MSG_MORIA_DEATH;

		/* dump the note*/
		add_monster_message(m_name, m_ptr, die_msg, FALSE);

		/* Generate treasure, etc */
		monster_death(m_ptr, FALSE);

		/* Delete the monster */
		delete_monster_idx(m_idx);

		mon_died = TRUE;
	}

	/* Damaged monster */
	else if (!is_mimicking(m_ptr))
	{
		/* Give detailed messages if visible or destroyed */
		if ((hurt_msg != MON_MSG_NONE) && seen)
		{
			add_monster_message(m_name, m_ptr, hurt_msg, FALSE);
		}

		/* Hack -- Pain message */
		else if (dam > 0) message_pain(m_ptr, dam);
	}

	return mon_died;
}

/**
 * Deal damage to a monster from the player
 *
 * This is a helper for project_m(). It isn't a type handler, but we take a handler context since that
 * has a lot of what we need.
 *
 * \param context is the project_m context.
 * \param m_name is the formatted monster name.
 * \return TRUE if the monster died, FALSE if it is still alive.
 */
static bool project_m_player_attack(project_monster_handler_context_t *context, const char *m_name)
{
	bool fear = FALSE;
	bool mon_died = FALSE;
	bool seen = context->seen;
	int dam = context->dam;
	enum mon_messages die_msg = context->die_msg;
	enum mon_messages hurt_msg = context->hurt_msg;
	monster_type *m_ptr = context->m_ptr;

	/*
	 * The monster is going to be killed, so display a specific death message before mon_take_hit() displays
	 * its own message that the player has killed/destroyed the monster. If the monster is not visible to
	 * the player, use a generic message.
	 */
	if (dam > m_ptr->hp) {
		if (!seen) die_msg = MON_MSG_MORIA_DEATH;
		add_monster_message(m_name, m_ptr, die_msg, FALSE);
	}

	mon_died = mon_take_hit(m_ptr, dam, &fear, "");

	/*
	 * If the monster didn't die, provide additional messages about how it was hurt/damaged. If a specific
	 * message isn't provided, display a message based on the amount of damage dealt. Also display a message
	 * if the hit caused the monster to flee.
	 */
	if (!mon_died) {
		if (seen && hurt_msg != MON_MSG_NONE)
			add_monster_message(m_name, m_ptr, hurt_msg, FALSE);
		else if (dam > 0)
			message_pain(m_ptr, dam);

		if (seen && fear)
			add_monster_message(m_name, m_ptr, MON_MSG_FLEE_IN_TERROR, TRUE);
	}

	return mon_died;
}

/**
 * Apply side effects from an attack onto a monster.
 *
 * This is a helper for project_m(). It isn't a type handler, but we take a handler context since that
 * has a lot of what we need.
 *
 * \param context is the project_m context.
 * \param m_name is the formatted monster name.
 * \param m_idx is the cave monster index.
 */
static void project_m_apply_side_effects(project_monster_handler_context_t *context, const char *m_name, int m_idx)
{
	int typ = context->type;
	monster_type *m_ptr = context->m_ptr;

	/*
	 * Handle side effects of an attack. First we check for polymorphing since it may not make sense to
	 * apply status effects to a changed monster. Right now, teleporting is also separate, but it could
	 * make sense in the future to change it so that we can apply other effects AND teleport the monster.
	 */
	if (context->do_poly) {
		enum mon_messages hurt_msg = MON_MSG_UNAFFECTED;
		const int x = context->x;
		const int y = context->y;
		int savelvl = typ == GF_OLD_POLY ? 11 : randint1(90);
		monster_race *old;
		monster_race *new;

		/* Uniques cannot be polymorphed */
		if (rf_has(m_ptr->race->flags, RF_UNIQUE)) {
			add_monster_message(m_name, m_ptr, hurt_msg, FALSE);
			return;
		}

		if (context->seen) context->obvious = TRUE;

		/* Saving throws are allowed */
		if (m_ptr->race->level > savelvl) {
			if (typ == GF_OLD_POLY) hurt_msg = MON_MSG_MAINTAIN_SHAPE;
			add_monster_message(m_name, m_ptr, hurt_msg, FALSE);
			return;
		}

		old = m_ptr->race;
		new = poly_race(old);

		/* Handle polymorph */
		if (new != old) {
			/* Report the polymorph before changing the monster */
			hurt_msg = MON_MSG_CHANGE;
			add_monster_message(m_name, m_ptr, hurt_msg, FALSE);

			/* Delete the old monster, and return a new one */
			delete_monster_idx(m_idx);
			place_new_monster(cave, y, x, new, FALSE, FALSE, ORIGIN_DROP_POLY);
			context->m_ptr = square_monster(cave, y, x);
		}
		else {
			add_monster_message(m_name, m_ptr, hurt_msg, FALSE);
		}
	}
	else if (context->teleport_distance > 0) {
		teleport_away(m_ptr, context->teleport_distance);
	}
	else {
		int i;

		/* Reduce stun if the monster is already stunned. */
		if (context->mon_timed[MON_TMD_STUN] > 0 && m_ptr->m_timed[MON_TMD_STUN] > 0) {
			context->mon_timed[MON_TMD_STUN] /= 2;
			context->mon_timed[MON_TMD_STUN] += 1;
		}

		/* Reroll confusion based on the provided amount. */
		if (context->mon_timed[MON_TMD_CONF] > 0) {
			context->mon_timed[MON_TMD_CONF] = damroll(3, (context->mon_timed[MON_TMD_CONF] / 2)) + 1;
		}

		/* If sleep is caused by the player, base the time on the player's level. */
		if (context->who == 0 && context->mon_timed[MON_TMD_SLEEP] > 0) {
			context->mon_timed[MON_TMD_SLEEP] = 500 + p_ptr->lev * 10;
		}

		for (i = 0; i < MON_TMD_MAX; i++) {
			if (context->mon_timed[i] > 0)
				context->obvious = mon_inc_timed(m_ptr, i, context->mon_timed[i], context->flag | MON_TMD_FLG_NOTIFY, context->id);
		}
	}
}

/*
 * Helper function for "project()" below.
 *
 * Handle a beam/bolt/ball causing damage to a monster.
 *
 * This routine takes a "source monster" (by index) which is mostly used to
 * determine if the player is causing the damage, and a "radius" (see below),
 * which is used to decrease the power of explosions with distance, and a
 * location, via integers which are modified by certain types of attacks
 * (polymorph and teleport being the obvious ones), a default damage, which
 * is modified as needed based on various properties, and finally a "damage
 * type" (see below).
 *
 * Note that this routine can handle "no damage" attacks (like teleport) by
 * taking a "zero" damage, and can even take "parameters" to attacks (like
 * confuse) by accepting a "damage", using it to calculate the effect, and
 * then setting the damage to zero.  Note that the "damage" parameter is
 * divided by the radius, so monsters not at the "epicenter" will not take
 * as much damage (or whatever)...
 *
 * Note that "polymorph" is dangerous, since a failure in "place_monster()"'
 * may result in a dereference of an invalid pointer.  XXX XXX XXX
 *
 * Various messages are produced, and damage is applied.
 *
 * Just "casting" a substance (i.e. plasma) does not make you immune, you must
 * actually be "made" of that substance, or "breathe" big balls of it.
 *
 * We assume that "Plasma" monsters, and "Plasma" breathers, are immune
 * to plasma.
 *
 * We assume "Nether" is an evil, necromantic force, so it doesn't hurt undead,
 * and hurts evil less.  If can breath nether, then it resists it as well.
 *
 * Damage reductions use the following formulas:
 *   Note that "dam = dam * 6 / (randint1(6) + 6);"
 *     gives avg damage of .655, ranging from .858 to .500
 *   Note that "dam = dam * 5 / (randint1(6) + 6);"
 *     gives avg damage of .544, ranging from .714 to .417
 *   Note that "dam = dam * 4 / (randint1(6) + 6);"
 *     gives avg damage of .444, ranging from .556 to .333
 *   Note that "dam = dam * 3 / (randint1(6) + 6);"
 *     gives avg damage of .327, ranging from .427 to .250
 *   Note that "dam = dam * 2 / (randint1(6) + 6);"
 *     gives something simple.
 *
 * In this function, "result" messages are postponed until the end, where
 * the "note" string is appended to the monster name, if not NULL.  So,
 * to make a spell have "no effect" just set "note" to NULL.  You should
 * also set "notice" to FALSE, or the player will learn what the spell does.
 *
 * We attempt to return "TRUE" if the player saw anything "obvious" happen.
 */
static bool project_m(int who, int r, int y, int x, int dam, int typ, bool obvious)
{
	monster_type *m_ptr;
	monster_lore *l_ptr;

	/* Is the monster "seen"? */
	bool seen = FALSE;
	bool mon_died = FALSE;

	/* Are we trying to id the source of this effect? */
	bool id = who < 0 ? !obvious : FALSE;

	/* Hold the monster name */
	char m_name[80];
	char m_poss[80];

	int m_idx = cave->m_idx[y][x];

	project_monster_handler_f monster_handler = gf_monster_handler(typ);
	project_monster_handler_context_t context = {
		who,
		r,
		y,
		x,
		dam,
		typ,
		seen,
		id,
		NULL, /* m_ptr */
		NULL, /* l_ptr */
		obvious,
		FALSE, /* skipped */
		0, /* flag */
		FALSE, /* do_poly */
		0, /* teleport_distance */
		MON_MSG_NONE, /* hurt_msg */
		MON_MSG_DIE, /* die_msg */
		{0, 0, 0, 0, 0, 0},
	};

	/* Walls protect monsters */
	if (!square_ispassable(cave, y,x)) return (FALSE);

	/* No monster here */
	if (!(m_idx > 0)) return (FALSE);

	/* Never affect projector */
	if (m_idx == who) return (FALSE);

	/* Obtain monster info */
	m_ptr = cave_monster(cave, m_idx);
	l_ptr = get_lore(m_ptr->race);
	context.m_ptr = m_ptr;
	context.l_ptr = l_ptr;

	if (m_ptr->ml) {
		seen = TRUE;
		context.seen = seen;
	}

	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);
	context.dam = dam;

	/* Get monster name and possessive here, in case of polymorphing. */
	monster_desc(m_name, sizeof(m_name), m_ptr, MDESC_DEFAULT);
	monster_desc(m_poss, sizeof(m_poss), m_ptr, MDESC_PRO_VIS | MDESC_POSS);

	/* Some monsters get "destroyed" */
	if (monster_is_unusual(m_ptr->race))
		context.die_msg = MON_MSG_DESTROYED;

	if (monster_handler != NULL)
		monster_handler(&context);

	/* Force obviousness for certain types if seen. */
	if (gf_force_obvious(typ) && context.seen)
		context.obvious = TRUE;

	dam = context.dam;
	obvious = context.obvious;

	/* Absolutely no effect */
	if (context.skipped) return (FALSE);

	/* Extract method of death, if the monster will be killed. */
	if (dam > m_ptr->hp)
		context.hurt_msg = context.die_msg;

	/* Apply damage to the monster, based on who did the damage. */
	if (who > 0)
		mon_died = project_m_monster_attack(&context, m_name, m_idx);
	else
		mon_died = project_m_player_attack(&context, m_name);

	if (!mon_died)
		project_m_apply_side_effects(&context, m_name, m_idx);

	/* Update locals again, since the project_m_* functions can change some values. */
	m_ptr = context.m_ptr;
	obvious = context.obvious;

	/* Verify this code XXX XXX XXX */
	/* Check for NULL, since polymorph can occasionally return NULL. */
	if (m_ptr != NULL) {
		/* Update the monster */
		if (!mon_died) update_mon(m_ptr, FALSE);

		/* Hack -- get new location in case of teleport */
		y = m_ptr->fy;
		x = m_ptr->fx;

		/* Redraw the monster grid */
		square_light_spot(cave, y, x);

		/* Update monster recall window */
		if (p_ptr->monster_race == m_ptr->race) {
			/* Window stuff */
			p_ptr->redraw |= (PR_MONSTER);
		}
	}

	/* Track it */
	project_m_n++;
	project_m_x = x;
	project_m_y = y;

	/* Return "Anything seen?" */
	return (obvious);
}

/*
 * Helper function for "project()" below.
 *
 * Handle a beam/bolt/ball causing damage to the player.
 *
 * This routine takes a "source monster" (by index), a "distance", a default
 * "damage", and a "damage type".  See "project_m()" above.
 *
 * If "rad" is non-zero, then the blast was centered elsewhere, and the damage
 * is reduced (see "project_m()" above).  This can happen if a monster breathes
 * at the player and hits a wall instead.
 *
 * We return "TRUE" if any "obvious" effects were observed.
 *
 * Actually, for historical reasons, we just assume that the effects were
 * obvious.  XXX XXX XXX
 */
static bool project_p(int who, int r, int y, int x, int dam, int typ, bool obvious)
{
	bool blind, seen;

	/* Get the damage type details */
	const struct gf_type *gf_ptr = &gf_table[typ];

	/* Source monster */
	monster_type *m_ptr;

	/* Monster name (for damage) */
	char killer[80];

	project_player_handler_f player_handler = gf_player_handler(typ);
	project_player_handler_context_t context = {
		who,
		r,
		y,
		x,
		dam,
		typ,
		obvious,
	};

	/* No player here */
	if (!(cave->m_idx[y][x] < 0)) return (FALSE);

	/* Never affect projector */
	if (cave->m_idx[y][x] == who) return (FALSE);

	/* Source monster */
	m_ptr = cave_monster(cave, who);

	/* Player blind-ness */
	blind = (p_ptr->timed[TMD_BLIND] ? TRUE : FALSE);

	/* Extract the "see-able-ness" */
	seen = (!blind && m_ptr->ml);

	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);

	/* Get the monster's real name */
	monster_desc(killer, sizeof(killer), m_ptr, MDESC_DIED_FROM);

	/* Let player know what is going on */
	if (!seen)
		msg("You are hit by %s!", gf_ptr->desc);

	if (player_handler != NULL)
		player_handler(&context);

	obvious = context.obvious;

	/* Adjust damage for resistance, immunity or vulnerability, and apply it */
	dam = adjust_dam(p_ptr, typ, dam, RANDOMISE, check_for_resist(p_ptr, typ,
		p_ptr->state.flags, TRUE));
	if (dam)
		take_hit(p_ptr, dam, killer);

	/* Disturb */
	disturb(p_ptr, 1, 0);

	/* Return "Anything seen?" */
	return (obvious);
}

/*
 * Generic "beam"/"bolt"/"ball" projection routine.
 *
 * Input:
 *   who: Index of "source" monster (negative for "player")
 *   rad: Radius of explosion (0 = beam/bolt, 1 to 9 = ball)
 *   y,x: Target location (or location to travel "towards")
 *   dam: Base damage roll to apply to affected monsters (or player)
 *   typ: Type of damage to apply to monsters (and objects)
 *   flg: Extra bit flags (see PROJECT_xxxx in "defines.h")
 *
 * Return:
 *   TRUE if any "effects" of the projection were observed, else FALSE
 *
 * Allows a monster (or player) to project a beam/bolt/ball of a given kind
 * towards a given location (optionally passing over the heads of interposing
 * monsters), and have it do a given amount of damage to the monsters (and
 * optionally objects) within the given radius of the final location.
 *
 * A "bolt" travels from source to target and affects only the target grid.
 * A "beam" travels from source to target, affecting all grids passed through.
 * A "ball" travels from source to the target, exploding at the target, and
 *   affecting everything within the given radius of the target location.
 *
 * Traditionally, a "bolt" does not affect anything on the ground, and does
 * not pass over the heads of interposing monsters, much like a traditional
 * missile, and will "stop" abruptly at the "target" even if no monster is
 * positioned there, while a "ball", on the other hand, passes over the heads
 * of monsters between the source and target, and affects everything except
 * the source monster which lies within the final radius, while a "beam"
 * affects every monster between the source and target, except for the casting
 * monster (or player), and rarely affects things on the ground.
 *
 * Two special flags allow us to use this function in special ways, the
 * "PROJECT_HIDE" flag allows us to perform "invisible" projections, while
 * the "PROJECT_JUMP" flag allows us to affect a specific grid, without
 * actually projecting from the source monster (or player).
 *
 * The player will only get "experience" for monsters killed by himself
 * Unique monsters can only be destroyed by attacks from the player
 *
 * Only 256 grids can be affected per projection, limiting the effective
 * "radius" of standard ball attacks to nine units (diameter nineteen).
 *
 * One can project in a given "direction" by combining PROJECT_THRU with small
 * offsets to the initial location (see "line_spell()"), or by calculating
 * "virtual targets" far away from the player.
 *
 * One can also use PROJECT_THRU to send a beam/bolt along an angled path,
 * continuing until it actually hits somethings (useful for "stone to mud").
 *
 * Bolts and Beams explode INSIDE walls, so that they can destroy doors.
 *
 * Balls must explode BEFORE hitting walls, or they would affect monsters
 * on both sides of a wall.  Some bug reports indicate that this is still
 * happening in 2.7.8 for Windows, though it appears to be impossible.
 *
 * We "pre-calculate" the blast area only in part for efficiency.
 * More importantly, this lets us do "explosions" from the "inside" out.
 * This results in a more logical distribution of "blast" treasure.
 * It also produces a better (in my opinion) animation of the explosion.
 * It could be (but is not) used to have the treasure dropped by monsters
 * in the middle of the explosion fall "outwards", and then be damaged by
 * the blast as it spreads outwards towards the treasure drop location.
 *
 * Walls and doors are included in the blast area, so that they can be
 * "burned" or "melted" in later versions.
 *
 * This algorithm is intended to maximize simplicity, not necessarily
 * efficiency, since this function is not a bottleneck in the code.
 *
 * We apply the blast effect from ground zero outwards, in several passes,
 * first affecting features, then objects, then monsters, then the player.
 * This allows walls to be removed before checking the object or monster
 * in the wall, and protects objects which are dropped by monsters killed
 * in the blast, and allows the player to see all affects before he is
 * killed or teleported away.  The semantics of this method are open to
 * various interpretations, but they seem to work well in practice.
 *
 * We process the blast area from ground-zero outwards to allow for better
 * distribution of treasure dropped by monsters, and because it provides a
 * pleasing visual effect at low cost.
 *
 * Note that the damage done by "ball" explosions decreases with distance.
 * This decrease is rapid, grids at radius "dist" take "1/dist" damage.
 *
 * Notice the "napalm" effect of "beam" weapons.  First they "project" to
 * the target, and then the damage "flows" along this beam of destruction.
 * The damage at every grid is the same as at the "center" of a "ball"
 * explosion, since the "beam" grids are treated as if they ARE at the
 * center of a "ball" explosion.
 *
 * Currently, specifying "beam" plus "ball" means that locations which are
 * covered by the initial "beam", and also covered by the final "ball", except
 * for the final grid (the epicenter of the ball), will be "hit twice", once
 * by the initial beam, and once by the exploding ball.  For the grid right
 * next to the epicenter, this results in 150% damage being done.  The center
 * does not have this problem, for the same reason the final grid in a "beam"
 * plus "bolt" does not -- it is explicitly removed.  Simply removing "beam"
 * grids which are covered by the "ball" will NOT work, as then they will
 * receive LESS damage than they should.  Do not combine "beam" with "ball".
 *
 * The array "gy[],gx[]" with current size "grids" is used to hold the
 * collected locations of all grids in the "blast area" plus "beam path".
 *
 * Note the rather complex usage of the "gm[]" array.  First, gm[0] is always
 * zero.  Second, for N>1, gm[N] is always the index (in gy[],gx[]) of the
 * first blast grid (see above) with radius "N" from the blast center.  Note
 * that only the first gm[1] grids in the blast area thus take full damage.
 * Also, note that gm[rad+1] is always equal to "grids", which is the total
 * number of blast grids.
 *
 * Note that once the projection is complete, (y2,x2) holds the final location
 * of bolts/beams, and the "epicenter" of balls.
 *
 * Note also that "rad" specifies the "inclusive" radius of projection blast,
 * so that a "rad" of "one" actually covers 5 or 9 grids, depending on the
 * implementation of the "distance" function.  Also, a bolt can be properly
 * viewed as a "ball" with a "rad" of "zero".
 *
 * Note that if no "target" is reached before the beam/bolt/ball travels the
 * maximum distance allowed (MAX_RANGE), no "blast" will be induced.  This
 * may be relevant even for bolts, since they have a "1x1" mini-blast.
 *
 * Note that for consistency, we "pretend" that the bolt actually takes "time"
 * to move from point A to point B, even if the player cannot see part of the
 * projection path.  Note that in general, the player will *always* see part
 * of the path, since it either starts at the player or ends on the player.
 *
 * Hack -- we assume that every "projection" is "self-illuminating".
 *
 * Hack -- when only a single monster is affected, we automatically track
 * (and recall) that monster, unless "PROJECT_JUMP" is used.
 *
 * Note that all projections now "explode" at their final destination, even
 * if they were being projected at a more distant destination.  This means
 * that "ball" spells will *always* explode.
 *
 * Note that we must call "handle_stuff(p_ptr)" after affecting terrain features
 * in the blast radius, in case the "illumination" of the grid was changed,
 * and "update_view()" and "update_monsters()" need to be called.
 */
bool project(int who, int rad, int y, int x, int dam, int typ, int flg)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int i, t, dist;

	int y1, x1;
	int y2, x2;

	int msec = op_ptr->delay_factor;

	/* Assume the player sees nothing */
	bool notice = FALSE;

	/* Assume the player has seen nothing */
	bool visual = FALSE;

	/* Assume the player has seen no blast grids */
	bool drawn = FALSE;

	/* Is the player blind? */
	bool blind = (p_ptr->timed[TMD_BLIND] ? TRUE : FALSE);

	/* Number of grids in the "path" */
	int path_n = 0;

	/* Actual grids in the "path" */
	u16b path_g[512];

	/* Number of grids in the "blast area" (including the "beam" path) */
	int grids = 0;

	/* Coordinates of the affected grids */
	byte gx[256], gy[256];

	/* Encoded "radius" info (see above) */
	byte gm[16];


	/* Hack -- Jump to target */
	if (flg & (PROJECT_JUMP))
	{
		x1 = x;
		y1 = y;

		/* Clear the flag */
		flg &= ~(PROJECT_JUMP);
	}

	/* Start at player */
	else if (who < 0)
	{
		x1 = px;
		y1 = py;
	}

	/* Start at monster */
	else if (who > 0)
	{
		x1 = cave_monster(cave, who)->fx;
		y1 = cave_monster(cave, who)->fy;
	}

	/* Oops */
	else
	{
		x1 = x;
		y1 = y;
	}


	/* Default "destination" */
	y2 = y;
	x2 = x;


	/* Hack -- verify stuff */
	if (flg & (PROJECT_THRU))
	{
		if ((x1 == x2) && (y1 == y2))
		{
			flg &= ~(PROJECT_THRU);
		}
	}


	/* Hack -- Assume there will be no blast (max radius 16) */
	for (dist = 0; dist < 16; dist++) gm[dist] = 0;


	/* Initial grid */
	y = y1;
	x = x1;

	/* Collect beam grids */
	if (flg & (PROJECT_BEAM))
	{
		gy[grids] = y;
		gx[grids] = x;
		grids++;
	}


	/* Calculate the projection path */
	path_n = project_path(path_g, MAX_RANGE, y1, x1, y2, x2, flg);


	/* Hack -- Handle stuff */
	handle_stuff(p_ptr);

	/* Project along the path */
	for (i = 0; i < path_n; ++i)
	{
		int oy = y;
		int ox = x;

		int ny = GRID_Y(path_g[i]);
		int nx = GRID_X(path_g[i]);

		/* Hack -- Balls explode before reaching walls */
		if (!square_ispassable(cave, ny, nx) && (rad > 0)) break;

		/* Advance */
		y = ny;
		x = nx;

		/* Collect beam grids */
		if (flg & (PROJECT_BEAM))
		{
			gy[grids] = y;
			gx[grids] = x;
			grids++;
		}

		/* Only do visuals if requested */
		if (!blind && !(flg & (PROJECT_HIDE)))
		{
			/* Only do visuals if the player can "see" the bolt */
			if (player_has_los_bold(y, x))
			{
				byte a;
				wchar_t c;

				/* Obtain the bolt pict */
				bolt_pict(oy, ox, y, x, typ, &a, &c);

				/* Visual effects */
				print_rel(c, a, y, x);
				move_cursor_relative(y, x);

				Term_fresh();
				if (p_ptr->redraw) redraw_stuff(p_ptr);

				Term_xtra(TERM_XTRA_DELAY, msec);

				square_light_spot(cave, y, x);

				Term_fresh();
				if (p_ptr->redraw) redraw_stuff(p_ptr);

				/* Display "beam" grids */
				if (flg & (PROJECT_BEAM))
				{
					/* Obtain the explosion pict */
					bolt_pict(y, x, y, x, typ, &a, &c);

					/* Visual effects */
					print_rel(c, a, y, x);
				}

				/* Hack -- Activate delay */
				visual = TRUE;
			}

			/* Hack -- delay anyway for consistency */
			else if (visual)
			{
				/* Delay for consistency */
				Term_xtra(TERM_XTRA_DELAY, msec);
			}
		}
	}


	/* Save the "blast epicenter" */
	y2 = y;
	x2 = x;

	/* Start the "explosion" */
	gm[0] = 0;

	/* Hack -- make sure beams get to "explode" */
	gm[1] = grids;

	/* Explode */
	/* Hack -- remove final beam grid */
	if (flg & (PROJECT_BEAM))
	{
		grids--;
	}

	/* Determine the blast area, work from the inside out */
	for (dist = 0; dist <= rad; dist++)
	{
		/* Scan the maximal blast area of radius "dist" */
		for (y = y2 - dist; y <= y2 + dist; y++)
		{
			for (x = x2 - dist; x <= x2 + dist; x++)
			{
				/* Ignore "illegal" locations */
				if (!square_in_bounds(cave, y, x)) continue;

				/* Enforce a "circular" explosion */
				if (distance(y2, x2, y, x) != dist) continue;

				/* Ball explosions are stopped by walls */
				if (!los(y2, x2, y, x)) continue;

				/* Save this grid */
				gy[grids] = y;
				gx[grids] = x;
				grids++;
			}
		}

		/* Encode some more "radius" info */
		gm[dist+1] = grids;
	}


	/* Speed -- ignore "non-explosions" */
	if (!grids) return (FALSE);


	/* Display the "blast area" if requested */
	if (!blind && !(flg & (PROJECT_HIDE)))
	{
		/* Then do the "blast", from inside out */
		for (t = 0; t <= rad; t++)
		{
			/* Dump everything with this radius */
			for (i = gm[t]; i < gm[t+1]; i++)
			{
				/* Extract the location */
				y = gy[i];
				x = gx[i];

				/* Only do visuals if the player can "see" the blast */
				if (player_has_los_bold(y, x))
				{
					byte a;
					wchar_t c;

					drawn = TRUE;

					/* Obtain the explosion pict */
					bolt_pict(y, x, y, x, typ, &a, &c);

					/* Visual effects -- Display */
					print_rel(c, a, y, x);
				}
			}

			/* Hack -- center the cursor */
			move_cursor_relative(y2, x2);

			/* Flush each "radius" separately */
			Term_fresh();

			/* Flush */
			if (p_ptr->redraw) redraw_stuff(p_ptr);

			/* Delay (efficiently) */
			if (visual || drawn)
			{
				Term_xtra(TERM_XTRA_DELAY, msec);
			}
		}

		/* Flush the erasing */
		if (drawn)
		{
			/* Erase the explosion drawn above */
			for (i = 0; i < grids; i++)
			{
				/* Extract the location */
				y = gy[i];
				x = gx[i];

				/* Hack -- Erase if needed */
				if (player_has_los_bold(y, x))
				{
					square_light_spot(cave, y, x);
				}
			}

			/* Hack -- center the cursor */
			move_cursor_relative(y2, x2);

			/* Flush the explosion */
			Term_fresh();

			/* Flush */
			if (p_ptr->redraw) redraw_stuff(p_ptr);
		}
	}


	/* Check features */
	if (flg & (PROJECT_GRID))
	{
		/* Start with "dist" of zero */
		dist = 0;

		/* Scan for features */
		for (i = 0; i < grids; i++)
		{
			/* Hack -- Notice new "dist" values */
			if (gm[dist+1] == i) dist++;

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Affect the feature in that grid */
			if (project_f(who, dist, y, x, dam, typ, FALSE)) notice = TRUE;
		}
	}


	/* Update stuff if needed */
	if (p_ptr->update) update_stuff(p_ptr);


	/* Check objects */
	if (flg & (PROJECT_ITEM))
	{
		/* Start with "dist" of zero */
		dist = 0;

		/* Scan for objects */
		for (i = 0; i < grids; i++)
		{
			/* Hack -- Notice new "dist" values */
			if (gm[dist+1] == i) dist++;

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Affect the object in the grid */
			if (project_o(who, dist, y, x, dam, typ, FALSE)) notice = TRUE;
		}
	}


	/* Check monsters */
	if (flg & (PROJECT_KILL))
	{
		/* Mega-Hack */
		project_m_n = 0;
		project_m_x = 0;
		project_m_y = 0;

		/* Start with "dist" of zero */
		dist = 0;

		/* Scan for monsters */
		for (i = 0; i < grids; i++)
		{
			/* Hack -- Notice new "dist" values */
			if (gm[dist+1] == i) dist++;

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Affect the monster in the grid */
			if (project_m(who, dist, y, x, dam, typ,
				(flg & PROJECT_AWARE ? TRUE : FALSE))) notice = TRUE;
		}

		/* Player affected one monster (without "jumping") */
		if ((who < 0) && (project_m_n == 1) && !(flg & (PROJECT_JUMP)))
		{
			/* Location */
			x = project_m_x;
			y = project_m_y;

			/* Track if possible */
			if (cave->m_idx[y][x] > 0)
			{
				monster_type *m_ptr = square_monster(cave, y, x);

				/* Hack -- auto-recall */
				if (m_ptr->ml) monster_race_track(m_ptr->race);

				/* Hack - auto-track */
				if (m_ptr->ml) health_track(p_ptr, m_ptr);
			}
		}
	}


	/* Check player */
	if (flg & (PROJECT_KILL))
	{
		/* Start with "dist" of zero */
		dist = 0;

		/* Scan for player */
		for (i = 0; i < grids; i++)
		{
			/* Hack -- Notice new "dist" values */
			if (gm[dist+1] == i) dist++;

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Affect the player (assume obvious) */
			if (project_p(who, dist, y, x, dam, typ, TRUE))
			{
				notice = TRUE;

				/* Only affect the player once */
				break;
			}
		}
	}


	/* Return "something was noticed" */
	return (notice);
}
