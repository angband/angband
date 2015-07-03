/**
 * \file project-player.c
 * \brief projection effects on the player
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
#include "init.h"
#include "mon-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"

/**
 * Adjust damage according to resistance or vulnerability.
 *
 * \param p is the player
 * \param type is the attack type we are checking.
 * \param dam is the unadjusted damage.
 * \param dam_aspect is the calc we want (min, avg, max, random).
 * \param resist is the degree of resistance (-1 = vuln, 3 = immune).
 */
int adjust_dam(struct player *p, int type, int dam, aspect dam_aspect, int resist)
{
	int i, denom;

	/* If an actual player exists, get their actual resist */
	if (p && p->race) {
		/* Ice is a special case */
		int res_type = (type == GF_ICE) ? GF_COLD: type;
		resist = p->state.el_info[res_type].res_level;

		/* Notice element stuff */
		equip_notice_element(p, res_type);
	}

	if (resist == 3) /* immune */
		return 0;

	/* Hack - acid damage is halved by armour, holy orb is halved */
	if ((type == GF_ACID && p && minus_ac(p)) || type == GF_HOLY_ORB)
		dam = (dam + 1) / 2;

	if (resist == -1) /* vulnerable */
		return (dam * 4 / 3);

	/* Variable resists vary the denominator, so we need to invert the logic
	 * of dam_aspect. (m_bonus is unused) */
	switch (dam_aspect) {
		case MINIMISE:
			denom = randcalc(gf_denom(type), 0, MAXIMISE);
			break;
		case MAXIMISE:
			denom = randcalc(gf_denom(type), 0, MINIMISE);
			break;
		case AVERAGE:
		case EXTREMIFY:
		case RANDOMISE:
			denom = randcalc(gf_denom(type), 0, dam_aspect);
			break;
		default:
			assert(0);
	}

	for (i = resist; i > 0; i--)
		if (denom)
			dam = dam * gf_num(type) / denom;

	return dam;
}


/**
 * ------------------------------------------------------------------------
 * Player handlers
 * ------------------------------------------------------------------------ */

/**
 * Drain stats at random
 *
 * \param num is the number of points to drain
 */
static void project_player_drain_stats(int num)
{
	int i, k = 0;
	const char *act = NULL;

	for (i = 0; i < num; i++) {
		switch (randint1(5)) {
			case 1: k = STAT_STR; act = "strong"; break;
			case 2: k = STAT_INT; act = "bright"; break;
			case 3: k = STAT_WIS; act = "wise"; break;
			case 4: k = STAT_DEX; act = "agile"; break;
			case 5: k = STAT_CON; act = "hale"; break;
		}

		msg("You're not as %s as you used to be...", act);
		player_stat_dec(player, k, FALSE);
	}

	return;
}

/**
 * Swap a random pair of stats
 */
static void project_player_swap_stats(void)
{
    int max1, cur1, max2, cur2, ii, jj;

    msg("Your body starts to scramble...");

    /* Pick a pair of stats */
    ii = randint0(STAT_MAX);
    for (jj = ii; jj == ii; jj = randint0(STAT_MAX)) /* loop */;

    max1 = player->stat_max[ii];
    cur1 = player->stat_cur[ii];
    max2 = player->stat_max[jj];
    cur2 = player->stat_cur[jj];

    player->stat_max[ii] = max2;
    player->stat_cur[ii] = cur2;
    player->stat_max[jj] = max1;
    player->stat_cur[jj] = cur1;

    player->upkeep->update |= (PU_BONUS);

    return;
}

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

static void project_player_handler_ACID(project_player_handler_context_t *context)
{
	if (player_is_immune(player, ELEM_ACID)) return;
	inven_damage(player, GF_ACID, MIN(context->dam * 5, 300));
}

static void project_player_handler_ELEC(project_player_handler_context_t *context)
{
	if (player_is_immune(player, ELEM_ELEC)) return;
	inven_damage(player, GF_ELEC, MIN(context->dam * 5, 300));
}

static void project_player_handler_FIRE(project_player_handler_context_t *context)
{
	if (player_is_immune(player, ELEM_FIRE)) return;
	inven_damage(player, GF_FIRE, MIN(context->dam * 5, 300));
}

static void project_player_handler_COLD(project_player_handler_context_t *context)
{
	if (player_is_immune(player, ELEM_COLD)) return;
	inven_damage(player, GF_COLD, MIN(context->dam * 5, 300));
}

static void project_player_handler_POIS(project_player_handler_context_t *context)
{
	if (!player_inc_timed(player, TMD_POISONED, 10 + randint1(context->dam),
						  TRUE, TRUE))
		msg("You resist the effect!");
}

static void project_player_handler_LIGHT(project_player_handler_context_t *context)
{
	if (player_resists(player, ELEM_LIGHT)) {
		msg("You resist the effect!");
		return;
	}

	(void)player_inc_timed(player, TMD_BLIND, 2 + randint1(5), TRUE, TRUE);
}

static void project_player_handler_DARK(project_player_handler_context_t *context)
{
	if (player_resists(player, ELEM_DARK)) {
		msg("You resist the effect!");
		return;
	}

	(void)player_inc_timed(player, TMD_BLIND, 2 + randint1(5), TRUE, TRUE);
}

static void project_player_handler_SOUND(project_player_handler_context_t *context)
{
	if (player_resists(player, ELEM_SOUND)) {
		msg("You resist the effect!");
		return;
	}

	/* Stun */
	if (!player_of_has(player, OF_PROT_STUN)) {
		int duration = 5 + randint1(context->dam / 3);
		if (duration > 35) duration = 35;
		(void)player_inc_timed(player, TMD_STUN, duration, TRUE, TRUE);
	}
}

static void project_player_handler_SHARD(project_player_handler_context_t *context)
{
	if (player_resists(player, ELEM_SHARD)) {
		msg("You resist the effect!");
		return;
	}

	/* Cuts */
	(void)player_inc_timed(player, TMD_CUT, randint1(context->dam), TRUE,
						   FALSE);
}

static void project_player_handler_NEXUS(project_player_handler_context_t *context)
{
	struct monster *mon = cave_monster(cave, context->who);

	if (player_resists(player, ELEM_NEXUS)) {
		msg("You resist the effect!");
		return;
	}

	/* Stat swap */
	if (one_in_(7)) {
		if (randint0(100) < player->state.skills[SKILL_SAVE]) {
			msg("You avoid the effect!");
			return;
		}
		project_player_swap_stats();
	} else if (one_in_(3)) { /* Teleport to */
		effect_simple(EF_TELEPORT_TO, "0", mon->fy, mon->fx, 0, NULL);
	} else if (one_in_(4)) { /* Teleport level */
		if (randint0(100) < player->state.skills[SKILL_SAVE]) {
			msg("You avoid the effect!");
			return;
		}
		effect_simple(EF_TELEPORT_LEVEL, "0", 0, 0, 0, NULL);
	} else { /* Teleport */
		const char *miles = "200";
		effect_simple(EF_TELEPORT, miles, 0, 1, 0, NULL);
	}
}

static void project_player_handler_NETHER(project_player_handler_context_t *context)
{
	int drain = 200 + (player->exp / 100) * z_info->life_drain_percent;

	if (player_resists(player, ELEM_NETHER) ||
		player_of_has(player, OF_HOLD_LIFE)) {
		msg("You resist the effect!");
		return;
	}

	/* Life draining */
	msg("You feel your life force draining away!");
	player_exp_lose(player, drain, FALSE);
}

static void project_player_handler_CHAOS(project_player_handler_context_t *context)
{
	if (player_resists(player, ELEM_CHAOS)) {
		msg("You resist the effect!");
		return;
	}

	/* Hallucination */
	(void)player_inc_timed(player, TMD_IMAGE, randint1(10), TRUE, FALSE);

	/* Confusion */
	(void)player_inc_timed(player, TMD_CONFUSED, 10 + randint0(20), TRUE, TRUE);

	/* Life draining */
	if (!player_of_has(player, OF_HOLD_LIFE)) {
		int drain = 5000 + (player->exp / 100) * z_info->life_drain_percent;
		msg("You feel your life force draining away!");
		player_exp_lose(player, drain, FALSE);
	}
}

static void project_player_handler_DISEN(project_player_handler_context_t *context)
{
	if (player_resists(player, ELEM_DISEN)) {
		msg("You resist the effect!");
		return;
	}

	/* Disenchant gear */
	effect_simple(EF_DISENCHANT, "0", 0, 0, 0, NULL);
}

static void project_player_handler_WATER(project_player_handler_context_t *context)
{
	/* Confusion */
	(void)player_inc_timed(player, TMD_CONFUSED, 5 + randint1(5), TRUE, TRUE);

	/* Stun */
	(void)player_inc_timed(player, TMD_STUN, randint1(40), TRUE, TRUE);
}

static void project_player_handler_ICE(project_player_handler_context_t *context)
{
	if (!player_is_immune(player, ELEM_COLD))
		inven_damage(player, GF_COLD, MIN(context->dam * 5, 300));

	/* Cuts */
	if (!player_resists(player, ELEM_SHARD))
		(void)player_inc_timed(player, TMD_CUT, damroll(5, 8), TRUE, FALSE);
	else
		msg("You resist the effect!");

	/* Stun */
	(void)player_inc_timed(player, TMD_STUN, randint1(15), TRUE, TRUE);
}

static void project_player_handler_GRAVITY(project_player_handler_context_t *context)
{
	msg("Gravity warps around you.");

	/* Blink */
	if (randint1(127) > player->lev) {
		const char *five = "5";
		effect_simple(EF_TELEPORT, five, 0, 1, 0, NULL);
	}

	/* Slow */
	(void)player_inc_timed(player, TMD_SLOW, 4 + randint0(4), TRUE, FALSE);

	/* Stun */
	if (!player_of_has(player, OF_PROT_STUN)) {
		int duration = 5 + randint1(context->dam / 3);
		if (duration > 35) duration = 35;
		(void)player_inc_timed(player, TMD_STUN, duration, TRUE, TRUE);
	}
}

static void project_player_handler_INERTIA(project_player_handler_context_t *context)
{
	/* Slow */
	(void)player_inc_timed(player, TMD_SLOW, 4 + randint0(4), TRUE, FALSE);
}

static void project_player_handler_FORCE(project_player_handler_context_t *context)
{
	char grids_away[5];

	/* Stun */
	(void)player_inc_timed(player, TMD_STUN, randint1(20), TRUE, TRUE);

	/* Thrust player away. */
	strnfmt(grids_away, sizeof(grids_away), "%d", 3 + context->dam / 20);
	effect_simple(EF_THRUST_AWAY, grids_away, context->y, context->x, 0, NULL);
}

static void project_player_handler_TIME(project_player_handler_context_t *context)
{
	if (one_in_(2)) {
		/* Life draining */
		int drain = 100 + (player->exp / 100) * z_info->life_drain_percent;
		msg("You feel your life force draining away!");
		player_exp_lose(player, drain, FALSE);
	} else if (!one_in_(5)) {
		/* Drain some stats */
		project_player_drain_stats(2);
	} else {
		/* Drain all stats */
		int i;
		msg("You're not as powerful as you used to be...");

		for (i = 0; i < STAT_MAX; i++)
			player_stat_dec(player, i, FALSE);
	}
}

static void project_player_handler_PLASMA(project_player_handler_context_t *context)
{
	/* Stun */
	if (!player_of_has(player, OF_PROT_STUN)) {
		int duration = 5 + randint1(context->dam * 3 / 4);
		if (duration > 35) duration = 35;
		(void)player_inc_timed(player, TMD_STUN, duration, TRUE, TRUE);
	}
}

static void project_player_handler_METEOR(project_player_handler_context_t *context)
{
}

static void project_player_handler_MISSILE(project_player_handler_context_t *context)
{
}

static void project_player_handler_MANA(project_player_handler_context_t *context)
{
}

static void project_player_handler_HOLY_ORB(project_player_handler_context_t *context)
{
}

static void project_player_handler_ARROW(project_player_handler_context_t *context)
{
}

static void project_player_handler_LIGHT_WEAK(project_player_handler_context_t *context)
{
}

static void project_player_handler_DARK_WEAK(project_player_handler_context_t *context)
{
	if (player_resists(player, ELEM_DARK)) {
		msg("You resist the effect!");
		return;
	}

	(void)player_inc_timed(player, TMD_BLIND, 3 + randint1(5), TRUE, TRUE);
}

static void project_player_handler_KILL_WALL(project_player_handler_context_t *context)
{
}

static void project_player_handler_KILL_DOOR(project_player_handler_context_t *context)
{
}

static void project_player_handler_KILL_TRAP(project_player_handler_context_t *context)
{
}

static void project_player_handler_MAKE_DOOR(project_player_handler_context_t *context)
{
}

static void project_player_handler_MAKE_TRAP(project_player_handler_context_t *context)
{
}

static const project_player_handler_f player_handlers[] = {
	#define ELEM(a, b, c, d, e, f, g, h, i, col) project_player_handler_##a,
	#include "list-elements.h"
	#undef ELEM
	#define PROJ_ENV(a, col, desc) project_player_handler_##a,
	#include "list-project-environs.h"
	#undef PROJ_ENV
	#define PROJ_MON(a, obv, desc) NULL, 
	#include "list-project-monsters.h"
	#undef PROJ_MON
	NULL
};

/**
 * Called from project() to affect the player
 *
 * Called for projections with the PROJECT_PLAY flag set, which includes
 * bolt, beam, ball and breath effects.
 *
 * \param who is the monster list index of the caster
 * \param r is the distance from the centre of the effect
 * \param y
 * \param x the coordinates of the grid being handled
 * \param dam is the "damage" from the effect at distance r from the centre
 * \param typ is the projection (GF_) type
 * \return whether the effects were obvious
 *
 * If "r" is non-zero, then the blast was centered elsewhere; the damage
 * is reduced in project() before being passed in here.  This can happen if a
 * monster breathes at the player and hits a wall instead.
 *
 * We assume the player is aware of some effect, and always return "TRUE".
 */
bool project_p(int who, int r, int y, int x, int dam, int typ)
{
	bool blind, seen;
	bool obvious = TRUE;

	/* Source monster */
	struct monster *mon;

	/* Monster name (for damage) */
	char killer[80];

	project_player_handler_f player_handler = player_handlers[typ];
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
	if (!(cave->squares[y][x].mon < 0)) return (FALSE);

	/* Never affect projector */
	if (cave->squares[y][x].mon == who) return (FALSE);

	/* Source monster */
	mon = cave_monster(cave, who);

	/* Player blind-ness */
	blind = (player->timed[TMD_BLIND] ? TRUE : FALSE);

	/* Extract the "see-able-ness" */
	seen = (!blind && mflag_has(mon->mflag, MFLAG_VISIBLE));

	/* Get the monster's real name */
	monster_desc(killer, sizeof(killer), mon, MDESC_DIED_FROM);

	/* Let player know what is going on */
	if (!seen)
		msg("You are hit by %s!", gf_blind_desc(typ));

	/* Adjust damage for resistance, immunity or vulnerability, and apply it */
	dam = adjust_dam(player, typ, dam, RANDOMISE,
					 player->state.el_info[typ].res_level);
	if (dam)
		take_hit(player, dam, killer);

	/* Handle side effects */
	if ((player_handler != NULL) && !player->is_dead)
		player_handler(&context);

	obvious = context.obvious;

	/* Disturb */
	disturb(player, 1);

	/* Return "Anything seen?" */
	return (obvious);
}

