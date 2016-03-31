/**
 * \file project-feat.c
 * \brief projection effects on terrain
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
#include "game-world.h"
#include "generate.h"
#include "obj-pile.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "trap.h"


/**
 * ------------------------------------------------------------------------
 * Feature handlers
 * ------------------------------------------------------------------------ */

typedef struct project_feature_handler_context_s {
	const int who;
	const int r;
	const int y;
	const int x;
	const int dam;
	const int type;
	bool obvious;
} project_feature_handler_context_t;
typedef void (*project_feature_handler_f)(project_feature_handler_context_t *);

/* Light up the grid */
static void project_feature_handler_LIGHT_WEAK(project_feature_handler_context_t *context)
{
	const int x = context->x;
	const int y = context->y;

	/* Turn on the light */
	sqinfo_on(cave->squares[y][x].info, SQUARE_GLOW);

	/* Grid is in line of sight */
	if (square_isview(cave, y, x)) {
		if (!player->timed[TMD_BLIND]) {
			/* Observe */
			context->obvious = true;
		}

		/* Fully update the visuals */
		player->upkeep->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);
	}
}

/* Darken the grid */
static void project_feature_handler_DARK_WEAK(project_feature_handler_context_t *context)
{
	const int x = context->x;
	const int y = context->y;

	if (player->depth != 0 || !is_daytime())
		/* Turn off the light */
		sqinfo_off(cave->squares[y][x].info, SQUARE_GLOW);

	/* Grid is in line of sight */
	if (square_isview(cave, y, x)) {
		/* Observe */
		context->obvious = true;

		/* Fully update the visuals */
		player->upkeep->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);
	}
}

/* Destroy walls (and doors) */
static void project_feature_handler_KILL_WALL(project_feature_handler_context_t *context)
{
	const int x = context->x;
	const int y = context->y;

	/* Non-walls (etc) */
	if (square_ispassable(cave, y, x)) return;

	/* Permanent walls */
	if (square_isperm(cave, y, x)) return;

	/* Different treatment for different walls */
	if (square_iswall(cave, y, x) && !square_hasgoldvein(cave, y, x)) {
		/* Message */
		if (square_isseen(cave, y, x)) {
			msg("The wall turns into mud!");
			context->obvious = true;

			/* Forget the wall */
			square_forget(cave, y, x);
		}

		/* Destroy the wall */
		square_destroy_wall(cave, y, x);
	} else if (square_iswall(cave, y, x) && square_hasgoldvein(cave, y, x)) {
		/* Message */
		if (square_isseen(cave, y, x)) {
			msg("The vein turns into mud!");
			msg("You have found something!");
			context->obvious = true;

			/* Forget the wall */
			square_forget(cave, y, x);
		}

		/* Destroy the wall */
		square_destroy_wall(cave, y, x);

		/* Place some gold */
		place_gold(cave, y, x, player->depth, ORIGIN_FLOOR);
	} else if (square_ismagma(cave, y, x) || square_isquartz(cave, y, x)) {
		/* Message */
		if (square_isseen(cave, y, x)) {
			msg("The vein turns into mud!");
			context->obvious = true;

			/* Forget the wall */
			square_forget(cave, y, x);
		}

		/* Destroy the wall */
		square_destroy_wall(cave, y, x);
	} else if (square_isrubble(cave, y, x)) {
		/* Message */
		if (square_isseen(cave, y, x)) {
			msg("The rubble turns into mud!");
			context->obvious = true;

			/* Forget the wall */
			square_forget(cave, y, x);
		}

		/* Destroy the rubble */
		square_destroy_rubble(cave, y, x);

		/* Hack -- place an object */
		if (randint0(100) < 10){
			if (square_isseen(cave, y, x)) {
				msg("There was something buried in the rubble!");
				context->obvious = true;
			}
			place_object(cave, y, x, player->depth, false, false,
						 ORIGIN_RUBBLE, 0);
		}
	} else if (square_isdoor(cave, y, x)) {
		/* Hack -- special message */
		if (square_isseen(cave, y, x)) {
			msg("The door turns into mud!");
			context->obvious = true;

			/* Forget the wall */
			square_forget(cave, y, x);
		}

		/* Destroy the feature */
		square_destroy_door(cave, y, x);
	}

	/* Update the visuals */
	player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Fully update the flow */
	player->upkeep->update |= (PU_FORGET_FLOW | PU_UPDATE_FLOW);
}

/* Destroy Doors */
static void project_feature_handler_KILL_DOOR(project_feature_handler_context_t *context)
{
	const int x = context->x;
	const int y = context->y;

	/* Destroy all doors */
	if (square_isdoor(cave, y, x)) {
		/* Check line of sight */
		if (square_isview(cave, y, x)) {
			/* Message */
			msg("There is a bright flash of light!");
			context->obvious = true;

			/* Visibility change */
			player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

			/* Forget the door */
			square_forget(cave, y, x);
		}

		/* Destroy the feature */
		square_destroy_door(cave, y, x);
	}
}

/* Disable traps, unlock doors */
static void project_feature_handler_KILL_TRAP(project_feature_handler_context_t *context)
{
	const int x = context->x;
	const int y = context->y;

	/* Reveal secret doors */
	if (square_issecretdoor(cave, y, x)) {
		place_closed_door(cave, y, x);

		/* Check line of sight */
		if (square_isseen(cave, y, x))
			context->obvious = true;
	}

	/* Disable traps, unlock doors */
	if (square_istrap(cave, y, x)) {
		/* Check line of sight */
		if (square_isview(cave, y, x)) {
			msg("The trap seizes up.");
			context->obvious = true;
		}

		/* Disable the trap */
		square_disable_trap(cave, y, x);
	} else if (square_islockeddoor(cave, y, x)) {
		/* Unlock the door */
		square_unlock_door(cave, y, x);

		/* Check line of sound */
		if (square_isview(cave, y, x)) {
			msg("Click!");
			context->obvious = true;
		}
	}
}

/* Make doors */
static void project_feature_handler_MAKE_DOOR(project_feature_handler_context_t *context)
{
	const int x = context->x;
	const int y = context->y;

	/* Require a grid without monsters */
	if (square_monster(cave, y, x) || square_isplayer(cave, y, x)) return;

	/* Require a floor grid */
	if (!square_isfloor(cave, y, x)) return;

	/* Push objects off the grid */
	if (square_object(cave, y, x))
		push_object(y,x);

	/* Create closed door */
	square_add_door(cave, y, x, true);

	/* Observe */
	if (square_isknown(cave, y, x))
		context->obvious = true;

	/* Update the visuals */
	player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
}

/* Make traps */
static void project_feature_handler_MAKE_TRAP(project_feature_handler_context_t *context)
{
	const int x = context->x;
	const int y = context->y;

	/* Require an "empty", non-warded floor grid */
	if (!square_isempty(cave, y, x)) return;
	if (square_iswarded(cave, y, x)) return;

	/* Create a trap, notice it */
	square_add_trap(cave, y, x);
	(void) square_reveal_trap(cave, y, x, false);
	context->obvious = true;
}

static void project_feature_handler_ACID(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_ELEC(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_FIRE(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_COLD(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_POIS(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

/* Light up the grid */
static void project_feature_handler_LIGHT(project_feature_handler_context_t *context)
{
	project_feature_handler_LIGHT_WEAK(context);
}

/* Darken the grid */
static void project_feature_handler_DARK(project_feature_handler_context_t *context)
{
	project_feature_handler_DARK_WEAK(context);
}

static void project_feature_handler_SOUND(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_SHARD(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_NEXUS(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_NETHER(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_CHAOS(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_DISEN(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_WATER(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_ICE(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_GRAVITY(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_INERTIA(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_FORCE(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_TIME(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_PLASMA(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_METEOR(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_MISSILE(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_MANA(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_HOLY_ORB(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_ARROW(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->y, context->x) &&
		!player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static const project_feature_handler_f feature_handlers[] = {
	#define ELEM(a, b, c, d, e, f, g, h, i, col) project_feature_handler_##a,
	#include "list-elements.h"
	#undef ELEM
	#define PROJ_ENV(a, col, desc) project_feature_handler_##a,
	#include "list-project-environs.h"
	#undef PROJ_ENV
	#define PROJ_MON(a, obv, desc) NULL, 
	#include "list-project-monsters.h"
	#undef PROJ_MON
	NULL
};

/**
 * Called from project() to affect terrain features
 *
 * Called for projections with the PROJECT_GRID flag set, which includes
 * beam, ball and breath effects.
 *
 * \param who is the monster list index of the caster
 * \param r is the distance from the centre of the effect
 * \param y the coordinates of the grid being handled
 * \param x the coordinates of the grid being handled
 * \param dam is the "damage" from the effect at distance r from the centre
 * \param typ is the projection (GF_) type
 * \return whether the effects were obvious
 *
 * Note that this function determines if the player can see anything that
 * happens by taking into account: blindness, line-of-sight, and illumination.
 *
 * Hack -- effects on grids which are memorized but not in view are also seen.
 */
bool project_f(int who, int r, int y, int x, int dam, int typ)
{
	bool obvious = false;

	project_feature_handler_context_t context = {
		who,
		r,
		y,
		x,
		dam,
		typ,
		obvious,
	};
	project_feature_handler_f feature_handler = feature_handlers[typ];

	if (feature_handler != NULL)
		feature_handler(&context);

	/* Return "Anything seen?" */
	return context.obvious;
}

