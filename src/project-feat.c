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
#include "source.h"
#include "trap.h"


/**
 * ------------------------------------------------------------------------
 * Feature handlers
 * ------------------------------------------------------------------------ */

typedef struct project_feature_handler_context_s {
	const struct source origin;
	const int r;
	const struct loc grid;
	const int dam;
	const int type;
	bool obvious;
} project_feature_handler_context_t;
typedef void (*project_feature_handler_f)(project_feature_handler_context_t *);

/* Light up the grid */
static void project_feature_handler_LIGHT_WEAK(project_feature_handler_context_t *context)
{
	const struct loc grid = context->grid;

	/* Turn on the light */
	sqinfo_on(square(cave, grid).info, SQUARE_GLOW);

	/* Grid is in line of sight */
	if (square_isview(cave, grid)) {
		if (!player->timed[TMD_BLIND]) {
			/* Observe */
			context->obvious = true;
		}

		/* Fully update the visuals */
		player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
	}
}

/* Darken the grid */
static void project_feature_handler_DARK_WEAK(project_feature_handler_context_t *context)
{
	const struct loc grid = context->grid;

	if ((player->depth != 0 || !is_daytime()) && !square_isbright(cave, grid)) {
		/* Turn off the light */
		sqinfo_off(square(cave, grid).info, SQUARE_GLOW);
	}

	/* Grid is in line of sight */
	if (square_isview(cave, grid)) {
		/* Observe */
		context->obvious = true;

		/* Fully update the visuals */
		player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
	}
}

/* Destroy walls (and doors) */
static void project_feature_handler_KILL_WALL(project_feature_handler_context_t *context)
{
	const struct loc grid = context->grid;

	/* Non-walls (etc) */
	if (square_ispassable(cave, grid) && !square_seemslikewall(cave, grid))
		return;

	/* Permanent walls */
	if (square_isperm(cave, grid)) return;

	/* Different treatment for different walls */
	if (square_isrubble(cave, grid)) {
		/* Message */
		if (square_isseen(cave, grid)) {
			msg("The rubble turns into mud!");
			context->obvious = true;

			/* Forget the wall */
			square_forget(cave, grid);
		}

		/* Destroy the rubble */
		square_destroy_rubble(cave, grid);

		/* Hack -- place an object */
		if (randint0(100) < 10){
			if (square_isseen(cave, grid)) {
				msg("There was something buried in the rubble!");
				context->obvious = true;
			}
			place_object(cave, grid, player->depth, false, false,
						 ORIGIN_RUBBLE, 0);
		}
	} else if (square_isdoor(cave, grid)) {
		/* Hack -- special message */
		if (square_isseen(cave, grid)) {
			msg("The door turns into mud!");
			context->obvious = true;

			/* Forget the wall */
			square_forget(cave, grid);
		}

		/* Destroy the feature */
		square_destroy_door(cave, grid);
	} else if (square_hasgoldvein(cave, grid)) {
		/* Message */
		if (square_isseen(cave, grid)) {
			msg("The vein turns into mud!");
			msg("You have found something!");
			context->obvious = true;

			/* Forget the wall */
			square_forget(cave, grid);
		}

		/* Destroy the wall */
		square_destroy_wall(cave, grid);

		/* Place some gold */
		place_gold(cave, grid, player->depth, ORIGIN_FLOOR);
	} else if (square_ismagma(cave, grid) || square_isquartz(cave, grid)) {
		/* Message */
		if (square_isseen(cave, grid)) {
			msg("The vein turns into mud!");
			context->obvious = true;

			/* Forget the wall */
			square_forget(cave, grid);
		}

		/* Destroy the wall */
		square_destroy_wall(cave, grid);
	} else if (square_iswall(cave, grid)) {
		/* Message */
		if (square_isseen(cave, grid)) {
			msg("The wall turns into mud!");
			context->obvious = true;

			/* Forget the wall */
			square_forget(cave, grid);
		}

		/* Destroy the wall */
		square_destroy_wall(cave, grid);
	}

	/* Update the visuals */
	player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
}

/* Destroy Doors */
static void project_feature_handler_KILL_DOOR(project_feature_handler_context_t *context)
{
	const struct loc grid = context->grid;

	/* Destroy all doors */
	if (square_isdoor(cave, grid)) {
		/* Check line of sight */
		if (square_isview(cave, grid)) {
			/* Message */
			msg("There is a bright flash of light!");
			context->obvious = true;

			/* Visibility change */
			player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

			/* Forget the door */
			square_forget(cave, grid);
		}

		/* Destroy the feature */
		square_destroy_door(cave, grid);
	}
}

/* Disable traps, unlock doors */
static void project_feature_handler_KILL_TRAP(project_feature_handler_context_t *context)
{
	const struct loc grid = context->grid;

	/* Reveal secret doors */
	if (square_issecretdoor(cave, grid)) {
		place_closed_door(cave, grid);

		/* Check line of sight */
		if (square_isseen(cave, grid))
			context->obvious = true;
	}

	/* Disable traps, unlock doors */
	if (square_isdisarmabletrap(cave, grid)) {
		/* Check line of sight */
		if (square_isview(cave, grid)) {
			msg("The trap seizes up.");
			context->obvious = true;
		}

		/* Disable the trap */
		square_disable_trap(cave, grid);

		/* Visibility change */
		player->upkeep->update |= (PU_UPDATE_VIEW);
	} else if (square_islockeddoor(cave, grid)) {
		/* Unlock the door */
		square_unlock_door(cave, grid);

		/* Check line of sound */
		if (square_isview(cave, grid)) {
			msg("Click!");
			context->obvious = true;
		}
	}
}

/* Make doors */
static void project_feature_handler_MAKE_DOOR(project_feature_handler_context_t *context)
{
	const struct loc grid = context->grid;

	/* Require a grid without monsters */
	if (square_monster(cave, grid) || square_isplayer(cave, grid)) return;

	/* Require a floor grid */
	if (!square_isfloor(cave, grid)) return;

	/* Push objects off the grid */
	if (square_object(cave, grid))
		push_object(grid);

	/* Create closed door */
	square_add_door(cave, grid, true);

	/* Observe */
	if (square_isknown(cave, grid))
		context->obvious = true;

	/* Update the visuals */
	player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
}

/* Make traps */
static void project_feature_handler_MAKE_TRAP(project_feature_handler_context_t *context)
{
	const struct loc grid = context->grid;

	/* Require an "empty" floor grid with no existing traps or glyphs */
	if (!square_isempty(cave, grid)) return;
	if (square_istrap(cave, grid)) return;

	/* Create a trap, try to notice it */
	if (one_in_(4)) {
		square_add_trap(cave, grid);
		(void) square_reveal_trap(cave, grid, false, false);
	}
	context->obvious = true;
}

static void project_feature_handler_ACID(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_ELEC(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_FIRE(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}

	/* Removes webs */
	if (square_iswebbed(cave, context->grid)) {
		square_destroy_trap(cave, context->grid);
	}

	/* Can create lava if extremely powerful. */
	if ((context->dam > randint1(1800) + 600) &&
		square_isfloor(cave, context->grid)) {
		/* Forget the floor, make lava. */
		square_unmark(cave, context->grid);
		square_set_feat(cave, context->grid, FEAT_LAVA);

		/* Objects that have survived should move */
		push_object(context->grid);
	}
}

static void project_feature_handler_COLD(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}

	/* Sufficiently intense cold can solidify lava. */
	if ((context->dam > randint1(900) + 300) &&
		square_isfiery(cave, context->grid)) {
		bool occupied = square_isoccupied(cave, context->grid);

		square_unmark(cave, context->grid);
		if (one_in_(2)) {
			square_set_feat(cave, context->grid, FEAT_FLOOR);
		} else if (one_in_(2) && !occupied) {
			square_set_feat(cave, context->grid, FEAT_RUBBLE);
		} else {
			square_set_feat(cave, context->grid, FEAT_PASS_RUBBLE);
		}
	}
}

static void project_feature_handler_POIS(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
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
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_SHARD(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_NEXUS(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_NETHER(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_CHAOS(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_DISEN(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_WATER(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_ICE(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}

	/* Sufficiently intense cold can solidify lava. */
	if ((context->dam > randint1(900) + 300) &&
		square_isfiery(cave, context->grid)) {
		bool occupied = square_isoccupied(cave, context->grid);

		square_unmark(cave, context->grid);
		if (one_in_(2)) {
			square_set_feat(cave, context->grid, FEAT_FLOOR);
		} else if (one_in_(2) && !occupied) {
			square_set_feat(cave, context->grid, FEAT_RUBBLE);
		} else {
			square_set_feat(cave, context->grid, FEAT_PASS_RUBBLE);
		}
	}
}

static void project_feature_handler_GRAVITY(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_INERTIA(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_FORCE(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_TIME(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_PLASMA(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}

	/* Can create lava if extremely powerful. */
	if ((context->dam > randint1(1800) + 600) &&
		square_isfloor(cave, context->grid)) {
		/* Forget the floor, make lava. */
		square_unmark(cave, context->grid);
		square_set_feat(cave, context->grid, FEAT_LAVA);

		/* Objects that have survived should move */
		push_object(context->grid);
	}
}

static void project_feature_handler_METEOR(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_MISSILE(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_MANA(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_HOLY_ORB(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_ARROW(project_feature_handler_context_t *context)
{
	/* Grid is in line of sight and player is not blind */
	if (square_isview(cave, context->grid) && !player->timed[TMD_BLIND]) {
		/* Observe */
		context->obvious = true;
	}
}

static void project_feature_handler_AWAY_UNDEAD(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_AWAY_EVIL(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_AWAY_SPIRIT(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_AWAY_ALL(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_TURN_UNDEAD(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_TURN_EVIL(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_TURN_LIVING(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_TURN_ALL(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_DISP_UNDEAD(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_DISP_EVIL(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_DISP_ALL(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_SLEEP_UNDEAD(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_SLEEP_EVIL(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_SLEEP_ALL(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_MON_CLONE(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_MON_POLY(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_MON_HEAL(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_MON_SPEED(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_MON_SLOW(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_MON_CONF(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_MON_HOLD(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_MON_STUN(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_MON_DRAIN(project_feature_handler_context_t *context)
{
}

static void project_feature_handler_MON_CRUSH(project_feature_handler_context_t *context)
{
}

static const project_feature_handler_f feature_handlers[] = {
	#define ELEM(a) project_feature_handler_##a,
	#include "list-elements.h"
	#undef ELEM
	#define PROJ(a) project_feature_handler_##a,
	#include "list-projections.h"
	#undef PROJ
	NULL
};

/**
 * Called from project() to affect terrain features
 *
 * Called for projections with the PROJECT_GRID flag set, which includes
 * beam, ball and breath effects.
 *
 * \param origin is the origin of the effect
 * \param r is the distance from the centre of the effect
 * \param y the coordinates of the grid being handled
 * \param x the coordinates of the grid being handled
 * \param dam is the "damage" from the effect at distance r from the centre
 * \param typ is the projection (PROJ_) type
 * \return whether the effects were obvious
 *
 * Note that this function determines if the player can see anything that
 * happens by taking into account: blindness, line-of-sight, and illumination.
 *
 * Hack -- effects on grids which are memorized but not in view are also seen.
 */
bool project_f(struct source origin, int r, struct loc grid, int dam, int typ)
{
	bool obvious = false;

	project_feature_handler_context_t context = {
		origin,
		r,
		grid,
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

