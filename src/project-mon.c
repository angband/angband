/**
 *  \file project-mon.c
 *  \brief projection effects on monsters
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
#include "effects.h"
#include "generate.h"
#include "mon-desc.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-move.h"
#include "mon-msg.h"
#include "mon-predicate.h"
#include "mon-spell.h"
#include "mon-timed.h"
#include "mon-util.h"
#include "player-calcs.h"
#include "project.h"
#include "source.h"


/**
 * Helper function -- return a "nearby" race for polymorphing
 *
 * Note that this function is one of the more "dangerous" ones...
 */
static struct monster_race *poly_race(struct monster_race *race)
{
	int i, minlvl, maxlvl, goal;

	assert(race && race->name);

	/* Uniques never polymorph */
	if (rf_has(race->flags, RF_UNIQUE)) return race;

	/* Allowable range of "levels" for resulting monster */
	goal = (player->depth + race->level) / 2 + 5;
	minlvl = MIN(race->level - 10, (race->level * 3) / 4);
	maxlvl = MAX(race->level + 10, (race->level * 5) / 4);

	/* Small chance to allow something really strong */
	if (one_in_(100)) maxlvl = 100;

	/* Try to pick a new, non-unique race within our level range */
	for (i = 0; i < 1000; i++) {
		struct monster_race *new_race = get_mon_num(goal);

		if (!new_race || new_race == race) continue;
		if (rf_has(new_race->flags, RF_UNIQUE)) continue;
		if (new_race->level < minlvl || new_race->level > maxlvl) continue;

		/* Avoid force-depth monsters, since it might cause a crash in project_m() */
		if (rf_has(new_race->flags, RF_FORCE_DEPTH) && player->depth < new_race->level) continue;

		return new_race;
	}

	/* If we get here, we weren't able to find a new race. */
	return race;
}

/**
 * Thrust the player or a monster away from the source of a projection.
 *
 * Monsters and players can be pushed past monsters or players weaker than
 * they are.
 */
void thrust_away(struct loc centre, struct loc target, int grids_away)
{
	struct loc grid, next;
	int i, d, first_d;
	int angle;

	/* Determine where target is in relation to caster, extend. */
	grid = loc_sum(loc_diff(target, centre), loc(20, 20));

	/* Find the angle (/2) of the line from caster to target. */
	angle = get_angle_to_grid[grid.y][grid.x];

	/* Start at the target grid. */
	grid = target;

	/* Up to the number of grids requested, force the target away from the
	 * source of the projection, until it hits something it can't travel
	 * around. */
	for (i = 0; i < grids_away; i++) {
		/* Randomize initial direction. */
		first_d = randint0(8);

		/* Look around (two possibilities for most angles). */
		for (d = first_d; d < 8 + first_d; d++) {
			/* Reject angles more than 44 degrees from desired direction. */
			if (d % 8 == 0) {	/* 135 */
				if ((angle > 157) || (angle < 114))
					continue;
			}
			if (d % 8 == 1) {	/* 45 */
				if ((angle > 66) || (angle < 23))
					continue;
			}
			if (d % 8 == 2) {	/* 0 */
				if ((angle > 21) && (angle < 159))
					continue;
			}
			if (d % 8 == 3) {	/* 90 */
				if ((angle > 112) || (angle < 68))
					continue;
			}
			if (d % 8 == 4) {	/* 158 */
				if ((angle > 179) || (angle < 136))
					continue;
			}
			if (d % 8 == 5) {	/* 113 */
				if ((angle > 134) || (angle < 91))
					continue;
			}
			if (d % 8 == 6) {	/* 22 */
				if ((angle > 44) || (angle < 1))
					continue;
			}
			if (d % 8 == 7) {	/* 67 */
				if ((angle > 89) || (angle < 46))
					continue;
			}

			/* Extract adjacent location */
			next = loc_sum(grid, ddgrid_ddd[d % 8]);

			/* There's someone there, try to switch places. */
			if (square(cave, next).mon != 0) {
				/* A monster is trying to pass. */
				if (square(cave, grid).mon > 0) {
					struct monster *mon = square_monster(cave, grid);
					if (square(cave, next).mon > 0) {
						struct monster *mon1 = square_monster(cave, next);

						/* Monsters cannot pass by stronger monsters. */
						if (mon1->race->mexp > mon->race->mexp)
							continue;
					} else {
						/* Monsters cannot pass by stronger characters. */
						if (player->lev * 2 > mon->race->level)
							continue;
					}
				}

				/* The player is trying to pass. */
				if (square(cave, grid).mon < 0) {
					if (square(cave, next).mon > 0) {
						struct monster *mon1 = square_monster(cave, next);

						/* Players cannot pass by stronger monsters. */
						if (mon1->race->level > player->lev * 2)
							continue;
					}
				}
			}

			/* Check for obstruction. */
			if (!square_isprojectable(cave, next)) {
				/* Some features allow entrance, but not exit. */
				if (square_ispassable(cave, next)) {
					/* Travel down the path. */
					monster_swap(grid, next);

					/* Jump to new location. */
					grid = next;

					/* We can't travel any more. */
					i = grids_away;

					/* Stop looking. */
					break;
				}

				/* If there are walls everywhere, stop here. */
				else if (d == (8 + first_d - 1)) {
					/* Message for player. */
					if (square(cave, grid).mon < 0)
						msg("You come to rest next to a wall.");
					i = grids_away;
				}
			} else {
				/* Travel down the path. */
				monster_swap(grid, next);

				/* Jump to new location. */
				grid = next;

				/* Stop looking at previous location. */
				break;
			}
		}
	}

	/* Some special messages or effects for player or monster. */
	if (square_isfiery(cave, grid)) {
		if (square(cave, grid).mon < 0) {
			msg("You are thrown into molten lava!");
		} else if (square(cave, grid).mon > 0) {
			struct monster *mon = square_monster(cave, grid);
			monster_take_terrain_damage(mon);
		}
	}

	/* Clear the projection mark. */
	sqinfo_off(square(cave, grid).info, SQUARE_PROJECT);
}

/**
 * ------------------------------------------------------------------------
 * Monster handlers
 * ------------------------------------------------------------------------ */

typedef struct project_monster_handler_context_s {
	const struct source origin;
	const int r;
	const struct loc grid;
	int dam;
	const int type;
	bool seen; /* Ideally, this would be const, but we can't with C89 initialization. */
	const bool id;
	struct monster *mon;
	struct monster_lore *lore;
	bool charm;
	bool obvious;
	bool skipped;
	u16b flag;
	int do_poly;
	int teleport_distance;
	enum mon_messages hurt_msg;
	enum mon_messages die_msg;
	int mon_timed[MON_TMD_MAX];
} project_monster_handler_context_t;
typedef void (*project_monster_handler_f)(project_monster_handler_context_t *);

static int adjust_radius(project_monster_handler_context_t *context, int amount)
{
	return (amount + context->r) / (context->r + 1);
}

/**
 * Resist an attack if the monster has the given elemental flag.
 *
 * If the effect is seen, we learn that the monster has a given flag.
 * Resistance is divided by the factor.
 *
 * \param context is the project_m context.
 * \param flag is the RF_ flag that the monster must have.
 * \param factor is the divisor for the base damage.
 */
static void project_monster_resist_element(project_monster_handler_context_t *context, int flag, int factor)
{
	if (context->seen) rf_on(context->lore->flags, flag);
	if (rf_has(context->mon->race->flags, flag)) {
		context->hurt_msg = MON_MSG_RESIST_A_LOT;
		context->dam /= factor;
	}
}

/**
 * Resist an attack if the monster has the given flag.
 *
 * If the effect is seen, we learn that the monster has a given flag.
 * Resistance is multiplied by the factor and reduced by a small random amount
 * (if reduce is set).
 *
 * \param context is the project_m context.
 * \param flag is the RF_ flag that the monster must have.
 * \param factor is the multiplier for the base damage.
 * \param reduce should be true if the base damage * factor should be reduced,
 * false if the base damage should be increased.
 * \param msg is the message that should be displayed when the monster is hurt.
 */
static void project_monster_resist_other(project_monster_handler_context_t *context, int flag, int factor, bool reduce, enum mon_messages msg)
{
	if (context->seen) rf_on(context->lore->flags, flag);
	if (rf_has(context->mon->race->flags, flag)) {
		context->hurt_msg = msg;
		context->dam *= factor;

		if (reduce)
			context->dam /= randint1(6) + 6;
	}
}

/**
 * Resist an attack if the monster has the given flag or hurt the monster
 * more if it has another flag.
 *
 * If the effect is seen, we learn the status of both flags. Resistance is
 * divided by imm_factor while hurt is multiplied by hurt_factor.
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
		rf_on(context->lore->flags, imm_flag);
		rf_on(context->lore->flags, hurt_flag);
	}

	if (rf_has(context->mon->race->flags, imm_flag)) {
		context->hurt_msg = MON_MSG_RESIST_A_LOT;
		context->dam /= imm_factor;
	}
	else if (rf_has(context->mon->race->flags, hurt_flag)) {
		context->hurt_msg = hurt_msg;
		context->die_msg = die_msg;
		context->dam *= hurt_factor;
	}
}

/**
 * Hurt the monster if it has a given flag or do no damage.
 *
 * If the effect is seen, we learn the status the flag. There is no damage
 * multiplier.
 *
 * \param context is the project_m context.
 * \param flag is the RF_ flag that the monster must have.
 * \param hurt_msg is the message that should be displayed when the monster is hurt.
 * \param die_msg is the message that should be displayed when the monster dies.
 */
static void project_monster_hurt_only(project_monster_handler_context_t *context, int flag, enum mon_messages hurt_msg, enum mon_messages die_msg)
{
	if (context->seen) rf_on(context->lore->flags, flag);

	if (rf_has(context->mon->race->flags, flag)) {
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
 * If the effect is seen, we learn that the monster has that spell (useful
 * for breaths). Resistance is multiplied by the factor and reduced by
 * a small random amount.
 *
 * \param context is the project_m context.
 * \param flag is the RSF_ flag that the monster must have.
 * \param factor is the multiplier for the base damage.
 */
static void project_monster_breath(project_monster_handler_context_t *context, int flag, int factor)
{
	if (rsf_has(context->mon->race->spell_flags, flag)) {
		/* Learn about breathers through resistance */
		if (context->seen) rsf_on(context->lore->spell_flags, flag);

		context->hurt_msg = MON_MSG_RESIST;
		context->dam *= factor;
		context->dam /= randint1(6) + 6;
	}
}

/**
 * Teleport away a monster that has a given flag.
 *
 * If the monster matches, it is teleported and the effect is obvious (if seen).
 * The player learns monster lore on whether or not the monster matches the
 * given flag if the effect is seen. Damage is not incurred by the monster.
 *
 * \param context is the project_m context.
 * \param flag is the RF_ flag that the monster must have.
 */
static void project_monster_teleport_away(project_monster_handler_context_t *context, int flag)
{
	if (context->seen) rf_on(context->lore->flags, flag);

	if (rf_has(context->mon->race->flags, flag)) {
		context->teleport_distance = context->dam;
		context->hurt_msg = MON_MSG_DISAPPEAR;
	} else {
		context->skipped = true;
	}

	context->obvious = true;
	context->dam = 0;
}

/**
 * Scare a monster that has a given flag.
 *
 * If the monster matches, fear is applied and the effect is obvious (if seen).
 * The player learns monster lore on whether or not the monster matches the
 * given flag if the effect is seen. Damage is not incurred by the monster.
 *
 * \param context is the project_m context.
 * \param flag is the RF_ flag that the monster must have.
 */
static void project_monster_scare(project_monster_handler_context_t *context, int flag)
{
    if (context->seen) rf_on(context->lore->flags, flag);

	if (rf_has(context->mon->race->flags, flag)) {
        context->mon_timed[MON_TMD_FEAR] = adjust_radius(context, context->dam);
	} else {
		context->skipped = true;
	}

	context->obvious = true;
	context->dam = 0;
}

/**
 * Dispel a monster that has a given flag.
 *
 * If the monster matches, damage is applied and the effect is obvious
 * (if seen). Otherwise, no damage is applied and the effect is not obvious.
 * The player learns monster lore on whether or not the monster matches the
 * given flag if the effect is seen.
 *
 * \param context is the project_m context.
 * \param flag is the RF_ flag that the monster must have.
 */
static void project_monster_dispel(project_monster_handler_context_t *context, int flag)
{
	if (context->seen) rf_on(context->lore->flags, flag);

	if (rf_has(context->mon->race->flags, flag)) {
		context->hurt_msg = MON_MSG_SHUDDER;
		context->die_msg = MON_MSG_DISSOLVE;
	} else {
		context->skipped = true;
		context->dam = 0;
	}

	context->obvious = true;
}

/**
 * Sleep a monster that has a given flag.
 *
 * If the monster matches, an attempt is made to put the monster to sleep
 * and the effect is obvious (if seen).
 * Otherwise, no attempt is made and the effect is not obvious.
 * The player learns monster lore on whether or not the monster matches the
 * given flag if the effect is seen.
 *
 * \param context is the project_m context.
 * \param flag is the RF_ flag that the monster must have.
 */
static void project_monster_sleep(project_monster_handler_context_t *context, int flag)
{
	if (context->seen && flag) rf_on(context->lore->flags, flag);

	if (flag && !rf_has(context->mon->race->flags, flag)) {
		context->skipped = true;
		context->dam = 0;
	}

	if (context->charm && rf_has(context->mon->race->flags, RF_ANIMAL)) {
		context->dam += context->dam / 2;
	}
	context->mon_timed[MON_TMD_SLEEP] = context->dam;
	context->dam = 0;

	context->obvious = true;
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
	project_monster_hurt_immune(context, RF_HURT_FIRE, RF_IM_FIRE, 2, 9, MON_MSG_CATCH_FIRE, MON_MSG_DISINTEGRATES);
}

/* Cold */
static void project_monster_handler_COLD(project_monster_handler_context_t *context)
{
	project_monster_hurt_immune(context, RF_HURT_COLD, RF_IM_COLD, 2, 9, MON_MSG_BADLY_FROZEN, MON_MSG_FREEZE_SHATTER);
}

/* Poison */
static void project_monster_handler_POIS(project_monster_handler_context_t *context)
{
	project_monster_resist_element(context, RF_IM_POIS, 9);
}

/* Light -- opposite of Dark */
static void project_monster_handler_LIGHT(project_monster_handler_context_t *context)
{
	if (context->seen) rf_on(context->lore->flags, RF_HURT_LIGHT);

	if (rsf_has(context->mon->race->spell_flags, RSF_BR_LIGHT)) {
		/* Learn about breathers through resistance */
		if (context->seen) rsf_on(context->lore->spell_flags, RSF_BR_LIGHT);

		context->hurt_msg = MON_MSG_RESIST;
		context->dam *= 2;
		context->dam /= randint1(6) + 6;
	}
	else if (rf_has(context->mon->race->flags, RF_HURT_LIGHT)) {
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

/* Sound -- Sound breathers resist */
static void project_monster_handler_SOUND(project_monster_handler_context_t *context)
{
	if (one_in_(3)) {
		context->mon_timed[MON_TMD_STUN] = adjust_radius(context, 5 + randint1(10));
	}

	project_monster_breath(context, RSF_BR_SOUN, 2);
}

/* Shards -- Shard breathers resist */
static void project_monster_handler_SHARD(project_monster_handler_context_t *context)
{
	project_monster_breath(context, RSF_BR_SHAR, 3);
}

/* Nexus */
static void project_monster_handler_NEXUS(project_monster_handler_context_t *context)
{
	project_monster_resist_other(context, RF_IM_NEXUS, 3, true, MON_MSG_RESIST);

	if (one_in_(3)) {
		/* Blink */
		context->teleport_distance = 10;
	} else if (one_in_(4)) {
		/* Teleport */
		context->teleport_distance = 50;
	}
}

/* Nether -- see above */
static void project_monster_handler_NETHER(project_monster_handler_context_t *context)
{
	/* Update the lore */
	if (context->seen) {
		/* Acquire knowledge of undead type and nether resistance */
		rf_on(context->lore->flags, RF_UNDEAD);
		rf_on(context->lore->flags, RF_IM_NETHER);

		/* If it isn't undead, acquire extra knowledge */
		if (!rf_has(context->mon->race->flags, RF_UNDEAD)) {
			/* Learn this creature breathes nether if true */
			if (rsf_has(context->mon->race->spell_flags, RSF_BR_NETH)) {
				rsf_on(context->lore->spell_flags, RSF_BR_NETH);
			}

			/* Otherwise learn about evil type */
			else {
				rf_on(context->lore->flags, RF_EVIL);
			}
		}
	}

	if (rf_has(context->mon->race->flags, RF_UNDEAD)) {
		context->hurt_msg = MON_MSG_IMMUNE;
		context->dam = 0;
	}
	else if (rf_has(context->mon->race->flags, RF_IM_NETHER)) {
		context->hurt_msg = MON_MSG_RESIST;
		context->dam *= 3;
		context->dam /= (randint1(6)+6);
	}
	else if (rf_has(context->mon->race->flags, RF_EVIL)) {
		context->dam /= 2;
		context->hurt_msg = MON_MSG_RESIST_SOMEWHAT;
	}
}

/* Chaos -- Chaos breathers resist */
static void project_monster_handler_CHAOS(project_monster_handler_context_t *context)
{
	/* Prevent polymorph on chaos breathers. */
	if (rsf_has(context->mon->race->spell_flags, RSF_BR_CHAO))
		context->do_poly = 0;
	else
		context->do_poly = 1;

	/* Hide resistance message (as assigned in project_monster_breath()). */
	context->mon_timed[MON_TMD_CONF] = adjust_radius(context, 10 + randint1(10));
	project_monster_breath(context, RSF_BR_CHAO, 3);
	context->hurt_msg = MON_MSG_NONE;
}

/* Disenchantment */
static void project_monster_handler_DISEN(project_monster_handler_context_t *context)
{
	project_monster_resist_other(context, RF_IM_DISEN, 3, true, MON_MSG_RESIST);

	/* Affect monsters which don't resist, and have non-innate spells */
	if (!rf_has(context->mon->race->flags, RF_IM_DISEN) &&
		monster_has_non_innate_spells(context->mon)) {
		context->mon_timed[MON_TMD_DISEN] = adjust_radius(context,
														  5 + randint1(10));
	}
}

/* Water damage */
static void project_monster_handler_WATER(project_monster_handler_context_t *context)
{
	/* Zero out the damage because this is an immunity flag. */
	project_monster_resist_other(context, RF_IM_WATER, 0, false, MON_MSG_IMMUNE);
}

/* Ice -- Cold + Stun */
static void project_monster_handler_ICE(project_monster_handler_context_t *context)
{
	if (one_in_(3)) {
		context->mon_timed[MON_TMD_STUN] = adjust_radius(context, 5 + randint1(10));
	}

	project_monster_hurt_immune(context, RF_HURT_COLD, RF_IM_COLD, 2, 9, MON_MSG_BADLY_FROZEN, MON_MSG_FREEZE_SHATTER);
}

/* Gravity -- breathers resist */
static void project_monster_handler_GRAVITY(project_monster_handler_context_t *context)
{
	/* Higher level monsters can resist the teleportation better */
	if (randint1(127) > context->mon->race->level)
		context->teleport_distance = 10;

	/* Prevent displacement on gravity breathers. */
	if (rsf_has(context->mon->race->spell_flags, RSF_BR_GRAV))
		context->teleport_distance = 0;

	project_monster_breath(context, RSF_BR_GRAV, 3);
}

/* Inertia -- breathers resist */
static void project_monster_handler_INERTIA(project_monster_handler_context_t *context)
{
	project_monster_breath(context, RSF_BR_INER, 3);
}

/* Force */
static void project_monster_handler_FORCE(project_monster_handler_context_t *context)
{
	struct loc centre = origin_get_loc(context->origin);

	if (one_in_(3)) {
		context->mon_timed[MON_TMD_STUN] = adjust_radius(context,
														 5 + randint1(10));
	}

	project_monster_breath(context, RSF_BR_WALL, 3);

	/* Prevent thrusting force breathers. */
	if (rsf_has(context->mon->race->spell_flags, RSF_BR_WALL))
		return;

	/* Thrust monster away */
	thrust_away(centre, context->grid, 3 + context->dam / 20);
}

/* Time -- breathers resist */
static void project_monster_handler_TIME(project_monster_handler_context_t *context)
{
	project_monster_breath(context, RSF_BR_TIME, 3);
}

/* Plasma */
static void project_monster_handler_PLASMA(project_monster_handler_context_t *context)
{
	project_monster_resist_other(context, RF_IM_PLASMA, 3, true, MON_MSG_RESIST);
}

static void project_monster_handler_METEOR(project_monster_handler_context_t *context)
{
}

static void project_monster_handler_MISSILE(project_monster_handler_context_t *context)
{
}

static void project_monster_handler_MANA(project_monster_handler_context_t *context)
{
}

/* Holy Orb -- hurts Evil */
static void project_monster_handler_HOLY_ORB(project_monster_handler_context_t *context)
{
	project_monster_resist_other(context, RF_EVIL, 2, false, MON_MSG_HIT_HARD);
}

static void project_monster_handler_ARROW(project_monster_handler_context_t *context)
{
}

/* Light, but only hurts susceptible creatures */
static void project_monster_handler_LIGHT_WEAK(project_monster_handler_context_t *context)
{
	project_monster_hurt_only(context, RF_HURT_LIGHT, MON_MSG_CRINGE_LIGHT, MON_MSG_SHRIVEL_LIGHT);
}

static void project_monster_handler_DARK_WEAK(project_monster_handler_context_t *context)
{
	context->skipped = true;
	context->dam = 0;
}

/* Stone to Mud */
static void project_monster_handler_KILL_WALL(project_monster_handler_context_t *context)
{
	project_monster_hurt_only(context, RF_HURT_ROCK, MON_MSG_LOSE_SKIN, MON_MSG_DISSOLVE);
}

static void project_monster_handler_KILL_DOOR(project_monster_handler_context_t *context)
{
	context->skipped = true;
	context->dam = 0;
}

static void project_monster_handler_KILL_TRAP(project_monster_handler_context_t *context)
{
	context->skipped = true;
	context->dam = 0;
}

static void project_monster_handler_MAKE_DOOR(project_monster_handler_context_t *context)
{
	context->skipped = true;
	context->dam = 0;
}

static void project_monster_handler_MAKE_TRAP(project_monster_handler_context_t *context)
{
	context->skipped = true;
	context->dam = 0;
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

/* Teleport evil (Use "dam" as "power") */
static void project_monster_handler_AWAY_SPIRIT(project_monster_handler_context_t *context)
{
	project_monster_teleport_away(context, RF_SPIRIT);
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

/* Turn living (Use "dam" as "power") */
static void project_monster_handler_TURN_LIVING(project_monster_handler_context_t *context)
{
    if (context->seen) {
		rf_on(context->lore->flags, RF_NONLIVING);
		rf_on(context->lore->flags, RF_UNDEAD);
	}

	if (monster_is_living(context->mon)) {
        context->mon_timed[MON_TMD_FEAR] = adjust_radius(context, context->dam);
	} else {
		context->skipped = true;
	}

	context->obvious = true;
	context->dam = 0;
}

/* Turn monster (Use "dam" as "power") */
static void project_monster_handler_TURN_ALL(project_monster_handler_context_t *context)
{
	context->mon_timed[MON_TMD_FEAR] = context->dam;
	context->dam = 0;
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

/* Sleep (Use "dam" as "power") */
static void project_monster_handler_SLEEP_UNDEAD(project_monster_handler_context_t *context)
{
	project_monster_sleep(context, RF_UNDEAD);
}

/* Sleep (Use "dam" as "power") */
static void project_monster_handler_SLEEP_EVIL(project_monster_handler_context_t *context)
{
	project_monster_sleep(context, RF_EVIL);
}

/* Sleep (Use "dam" as "power") */
static void project_monster_handler_SLEEP_ALL(project_monster_handler_context_t *context)
{
	project_monster_sleep(context, RF_NONE);
}

/* Clone monsters (Ignore "dam") */
static void project_monster_handler_MON_CLONE(project_monster_handler_context_t *context)
{
	/* Heal fully */
	context->mon->hp = context->mon->maxhp;

	/* Speed up */
	mon_inc_timed(context->mon, MON_TMD_FAST, 50, MON_TMD_FLG_NOTIFY);

	/* Attempt to clone. */
	if (multiply_monster(cave, context->mon))
		context->hurt_msg = MON_MSG_SPAWN;

	/* No "real" damage */
	context->dam = 0;

}

/* Polymorph monster (Use "dam" as "power") */
static void project_monster_handler_MON_POLY(project_monster_handler_context_t *context)
{
	if (context->charm && rf_has(context->mon->race->flags, RF_ANIMAL)) {
		context->dam += context->dam / 2;
	}
	/* Polymorph later */
	context->do_poly = context->dam;

	/* No "real" damage */
	context->dam = 0;
}

/* Heal Monster (use "dam" as amount of healing) */
static void project_monster_handler_MON_HEAL(project_monster_handler_context_t *context)
{
	/* Wake up, become aware */
	monster_wake(context->mon, false, 100);

	/* Heal */
	context->mon->hp += context->dam;

	/* No overflow */
	if (context->mon->hp > context->mon->maxhp)
		context->mon->hp = context->mon->maxhp;

	/* Redraw (later) if needed */
	if (player->upkeep->health_who == context->mon)
		player->upkeep->redraw |= (PR_HEALTH);

	/* Message */
	else context->hurt_msg = MON_MSG_HEALTHIER;

	/* No "real" damage */
	context->dam = 0;
}

/* Speed Monster (Ignore "dam") */
static void project_monster_handler_MON_SPEED(project_monster_handler_context_t *context)
{
	context->mon_timed[MON_TMD_FAST] = context->dam;
	context->dam = 0;
}

/* Slow Monster (Use "dam" as "power") */
static void project_monster_handler_MON_SLOW(project_monster_handler_context_t *context)
{
	if (context->charm && rf_has(context->mon->race->flags, RF_ANIMAL)) {
		context->dam += context->dam / 2;
	}
	context->mon_timed[MON_TMD_SLOW] = context->dam;
	context->dam = 0;
}

/* Confusion (Use "dam" as "power") */
static void project_monster_handler_MON_CONF(project_monster_handler_context_t *context)
{
	if (context->charm && rf_has(context->mon->race->flags, RF_ANIMAL)) {
		context->dam += context->dam / 2;
	}
	context->mon_timed[MON_TMD_CONF] = context->dam;
	context->dam = 0;
}

/* Hold (Use "dam" as "power") */
static void project_monster_handler_MON_HOLD(project_monster_handler_context_t *context)
{
	if (context->charm && rf_has(context->mon->race->flags, RF_ANIMAL)) {
		context->dam += context->dam / 2;
	}
	context->mon_timed[MON_TMD_HOLD] = context->dam;
	context->dam = 0;
}

/* Stun (Use "dam" as "power") */
static void project_monster_handler_MON_STUN(project_monster_handler_context_t *context)
{
	if (context->charm && rf_has(context->mon->race->flags, RF_ANIMAL)) {
		context->dam += context->dam / 2;
	}
	context->mon_timed[MON_TMD_STUN] = context->dam;
	context->dam = 0;
}

/* Drain Life */
static void project_monster_handler_MON_DRAIN(project_monster_handler_context_t *context)
{
	if (context->seen) context->obvious = true;
	if (context->seen) {
		rf_on(context->lore->flags, RF_UNDEAD);
	}
	if (monster_is_nonliving(context->mon)) {
		context->hurt_msg = MON_MSG_UNAFFECTED;
		context->obvious = false;
		context->dam = 0;
	}
}

/* Crush */
static void project_monster_handler_MON_CRUSH(project_monster_handler_context_t *context)
{
	if (context->seen) context->obvious = true;
	if (context->mon->hp >= context->dam) {
		context->hurt_msg = MON_MSG_UNAFFECTED;
		context->obvious = false;
		context->skipped = true;
		context->dam = 0;
	}
}

static const project_monster_handler_f monster_handlers[] = {
	#define ELEM(a) project_monster_handler_##a,
	#include "list-elements.h"
	#undef ELEM
	#define PROJ(a) project_monster_handler_##a,
	#include "list-projections.h"
	#undef PROJ
	NULL
};


/**
 * Deal damage to a monster from another monster.
 *
 * This is a helper for project_m(). It is very similar to mon_take_hit(),
 * but eliminates the player-oriented stuff of that function. It isn't a type
 * handler, but we take a handler context since that has a lot of what we need.
 *
 * \param context is the project_m context.
 * \param m_idx is the cave monster index.
 * \return true if the monster died, false if it is still alive.
 */
static bool project_m_monster_attack(project_monster_handler_context_t *context, int m_idx)
{
	bool mon_died = false;
	bool seen = context->seen;
	int dam = context->dam;
	enum mon_messages die_msg = context->die_msg;
	enum mon_messages hurt_msg = context->hurt_msg;
	struct monster *mon = context->mon;

	/* "Unique" monsters can only be "killed" by the player */
	if (rf_has(mon->race->flags, RF_UNIQUE)) {
		/* Reduce monster hp to zero, but don't kill it. */
		if (dam > mon->hp) dam = mon->hp;
	}

	/* Redraw (later) if needed */
	if (player->upkeep->health_who == mon)
		player->upkeep->redraw |= (PR_HEALTH);

	/* Wake the monster up, don't notice the player */
	monster_wake(mon, false, 0);

	/* Hurt the monster */
	mon->hp -= dam;

	/* Dead or damaged monster */
	if (mon->hp < 0) {
		/* Give detailed messages if destroyed */
		if (!seen) die_msg = MON_MSG_MORIA_DEATH;

		/* Death message */
		add_monster_message(mon, die_msg, false);

		/* Generate treasure, etc */
		monster_death(mon, false);

		/* Delete the monster */
		delete_monster_idx(m_idx);

		mon_died = true;
	} else if (!monster_is_mimicking(mon)) {
		/* Give detailed messages if visible or destroyed */
		if ((hurt_msg != MON_MSG_NONE) && seen)
			add_monster_message(mon, hurt_msg, false);

		/* Hack -- Pain message */
		else if (dam > 0)
			message_pain(mon, dam);
	}

	return mon_died;
}

/**
 * Deal damage to a monster from a non-monster source (usually a player,
 * but could also be a trap)
 *
 * This is a helper for project_m(). It isn't a type handler, but we take a
 * handler context since that has a lot of what we need.
 *
 * \param context is the project_m context.
 * \return true if the monster died, false if it is still alive.
 */
static bool project_m_player_attack(project_monster_handler_context_t *context)
{
	bool fear = false;
	bool mon_died = false;
	bool seen = context->seen;
	int dam = context->dam;
	enum mon_messages die_msg = context->die_msg;
	enum mon_messages hurt_msg = context->hurt_msg;
	struct monster *mon = context->mon;

	/* The monster is going to be killed, so display a specific death message.
	 * If the monster is not visible to the player, use a generic message.
	 *
	 * Note that mon_take_hit() below is passed a zero-length string, which
	 * ensures it doesn't print any death message and allows correct ordering
	 * of messages. */
	if (dam > mon->hp) {
		if (!seen) die_msg = MON_MSG_MORIA_DEATH;
		add_monster_message(mon, die_msg, false);
	}

	/* No damage is now going to mean the monster is not hit - and hence
	 * is not woken or released from holding */
	if(dam) {
		mon_died = mon_take_hit(mon, dam, &fear, "");
	}

	/* If the monster didn't die, provide additional messages about how it was
	 * hurt/damaged. If a specific message isn't provided, display a message
	 * based on the amount of damage dealt. Also display a message
	 * if the hit caused the monster to flee. */
	if (!mon_died) {
		if (seen && hurt_msg != MON_MSG_NONE)
			add_monster_message(mon, hurt_msg, false);
		else if (dam > 0)
			message_pain(mon, dam);

		if (seen && fear)
			add_monster_message(mon, MON_MSG_FLEE_IN_TERROR, true);
	}

	return mon_died;
}

/**
 * Apply side effects from an attack onto a monster.
 *
 * This is a helper for project_m(). It isn't a type handler, but we take a
 * handler context since that has a lot of what we need.
 *
 * \param context is the project_m context.
 * \param m_idx is the cave monster index.
 */
static void project_m_apply_side_effects(project_monster_handler_context_t *context, int m_idx)
{
	int typ = context->type;
	struct monster *mon = context->mon;

	/*
	 * Handle side effects of an attack. First we check for polymorphing since
	 * it may not make sense to apply status effects to a changed monster.
	 * Right now, teleporting is also separate, but it could make sense in the
	 * future to change it so that we can apply other effects AND teleport the
	 * monster.
	 */
	if (context->do_poly) {
		enum mon_messages hurt_msg = MON_MSG_UNAFFECTED;
		const struct loc grid = context->grid;
		int savelvl = 0;
		struct monster_race *old;
		struct monster_race *new;

		/* Uniques cannot be polymorphed */
		if (rf_has(mon->race->flags, RF_UNIQUE)) {
			add_monster_message(mon, hurt_msg, false);
			return;
		}

		if (context->seen) context->obvious = true;

		/* Saving throws depend on damage for direct poly, random for chaos */
		if (typ == PROJ_MON_POLY)
			savelvl = randint1(MAX(1, context->do_poly - 10)) + 10;
		else
			savelvl = randint1(90);
		if (mon->race->level > savelvl) {
			if (typ == PROJ_MON_POLY) hurt_msg = MON_MSG_MAINTAIN_SHAPE;
			add_monster_message(mon, hurt_msg, false);
			return;
		}

		old = mon->race;
		new = poly_race(old);

		/* Handle polymorph */
		if (new != old) {
			struct monster_group_info info = {0, 0 };

			/* Report the polymorph before changing the monster */
			hurt_msg = MON_MSG_CHANGE;
			add_monster_message(mon, hurt_msg, false);

			/* Delete the old monster, and return a new one */
			delete_monster_idx(m_idx);
			place_new_monster(cave, grid, new, false, false, info,
							  ORIGIN_DROP_POLY);
			context->mon = square_monster(cave, grid);
		} else {
			add_monster_message(mon, hurt_msg, false);
		}
	} else if (context->teleport_distance > 0) {
		char dice[5];
		strnfmt(dice, sizeof(dice), "%d", context->teleport_distance);
		effect_simple(EF_TELEPORT, context->origin, dice, 0, 0, 0,
					  context->grid.y, context->grid.x, NULL);

		/* Wake the monster up, don't notice the player */
		monster_wake(mon, false, 0);
	} else {
		for (int i = 0; i < MON_TMD_MAX; i++) {
			if (context->mon_timed[i] > 0) {
				mon_inc_timed(mon,
							  i,
							  context->mon_timed[i],
							  context->flag | MON_TMD_FLG_NOTIFY);
				context->obvious = true;
			}
		}
	}
}

/**
 * Called from project() to affect monsters
 *
 * Called for projections with the PROJECT_KILL flag set, which includes
 * bolt, beam, ball and breath effects.
 *
 * \param origin is the monster list index of the caster
 * \param r is the distance from the centre of the effect
 * \param y the coordinates of the grid being handled
 * \param x the coordinates of the grid being handled
 * \param dam is the "damage" from the effect at distance r from the centre
 * \param typ is the projection (PROJ_) type
 * \param flg consists of any relevant PROJECT_ flags
 * \return whether the effects were obvious
 *
 * Note that this routine can handle "no damage" attacks (like teleport) by
 * taking a zero damage, and can even take parameters to attacks (like
 * confuse) by accepting a "damage", using it to calculate the effect, and
 * then setting the damage to zero.  Note that actual damage should be already 
 * adjusted for distance from the "epicenter" when passed in, but other effects 
 * may be influenced by r.
 *
 * Note that "polymorph" is dangerous, since a failure in "place_monster()"'
 * may result in a dereference of an invalid pointer.  XXX XXX XXX
 *
 * Various messages are produced, and damage is applied.
 *
 * Just casting an element (e.g. plasma) does not make you immune, you must
 * actually be made of that substance, or breathe big balls of it.
 *
 * We assume that "Plasma" monsters, and "Plasma" breathers, are immune
 * to plasma.
 *
 * We assume "Nether" is an evil, necromantic force, so it doesn't hurt undead,
 * and hurts evil less.  If can breath nether, then it resists it as well.
 * This should actually be coded into monster records rather than aasumed - NRM
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
 * to make a spell have no effect just set "note" to NULL.  You should
 * also set "notice" to false, or the player will learn what the spell does.
 *
 * Note that this function determines if the player can see anything that
 * happens by taking into account: blindness, line-of-sight, and illumination.
 *
 * Hack -- effects on grids which are memorized but not in view are also seen.
 */
void project_m(struct source origin, int r, struct loc grid, int dam, int typ,
			   int flg, bool *did_hit, bool *was_obvious)
{
	struct monster *mon;
	struct monster_lore *lore;

	/* Is the monster "seen"? */
	bool seen = false;
	bool mon_died = false;

	/* Is the effect obvious? */
	bool obvious = (flg & PROJECT_AWARE ? true : false);

	/* Are we trying to id the source of this effect? */
	bool id = (origin.what == SRC_PLAYER) ? !obvious : false;

	/* Is the source an extra charming player? */
	bool charm = (origin.what == SRC_PLAYER) ?
		player_has(player, PF_CHARM) : false;

	int m_idx = square(cave, grid).mon;

	project_monster_handler_f monster_handler = monster_handlers[typ];
	project_monster_handler_context_t context = {
		origin,
		r,
		grid,
		dam,
		typ,
		seen,
		id,
		NULL, /* mon */
		NULL, /* lore */
		charm,
		obvious,
		false, /* skipped */
		0, /* flag */
		0, /* do_poly */
		0, /* teleport_distance */
		MON_MSG_NONE, /* hurt_msg */
		MON_MSG_DIE, /* die_msg */
		{0, 0, 0, 0, 0, 0},
	};

	*did_hit = false;
	*was_obvious = false;

	/* Walls protect monsters */
	if (!square_ispassable(cave, grid)) return;

	/* No monster here */
	if (!(m_idx > 0)) return;

	/* Never affect projector */
	if (origin.what == SRC_MONSTER && origin.which.monster == m_idx) return;

	/* Obtain monster info */
	mon = cave_monster(cave, m_idx);
	lore = get_lore(mon->race);
	context.mon = mon;
	context.lore = lore;

	/* See visible monsters */
	if (monster_is_visible(mon)) {
		seen = true;
		context.seen = seen;
	}

	/* Breathers may not blast members of the same race. */
	if (origin.what == SRC_MONSTER && (flg & PROJECT_SAFE)) {
		/* Point to monster information of caster */
		struct monster *caster = cave_monster(cave, origin.which.monster);

		/* Skip monsters with the same race */
		if (caster->race == mon->race)
			return;
	}

	/* Some monsters get "destroyed" */
	if (monster_is_destroyed(mon))
		context.die_msg = MON_MSG_DESTROYED;

	/* Force obviousness for certain types if seen. */
	if (projections[typ].obvious && context.seen)
		context.obvious = true;

	if (monster_handler != NULL)
		monster_handler(&context);

	dam = context.dam;
	obvious = context.obvious;

	/* Absolutely no effect */
	if (context.skipped) return;

	/* Apply damage to the monster, based on who did the damage. */
	if (origin.what == SRC_MONSTER) {
		mon_died = project_m_monster_attack(&context, m_idx);
	} else {
		mon_died = project_m_player_attack(&context);
	}

	if (!mon_died)
		project_m_apply_side_effects(&context, m_idx);

	/* Update locals again, since the project_m_* functions can change
	 * some values. */
	mon = context.mon;
	obvious = context.obvious;

	/* Check for NULL, since polymorph can occasionally return NULL. */
	if (mon != NULL) {
		/* Update the monster */
		if (!mon_died)
			update_mon(mon, cave, false);

		/* Redraw the (possibly new) monster grid */
		square_light_spot(cave, mon->grid);

		/* Update monster recall window */
		if (player->upkeep->monster_race == mon->race) {
			/* Window stuff */
			player->upkeep->redraw |= (PR_MONSTER);
		}
	}

	/* Track it */
	*did_hit = true;

	/* Return "Anything seen?" */
	*was_obvious = !!obvious;
}

