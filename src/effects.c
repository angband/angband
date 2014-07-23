/*
 * File: effects.c
 * Purpose: Big switch statement for every effect in the game
 *
 * Copyright (c) 2007 Andi Sidwell
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
#include "dungeon.h"
#include "generate.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-spell.h"
#include "mon-util.h"
#include "obj-chest.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-ignore.h"
#include "obj-power.h"
#include "obj-tval.h"
#include "obj-ui.h"
#include "obj-util.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"
#include "spells.h"
#include "tables.h"
#include "target.h"
#include "trap.h"


typedef struct effect_handler_context_s {
	const atomic_effect_index effect;
	const bool aware;
	const int dir;
	const int beam;
	const int boost;
	const random_value value;
	const int p1, p2;
	bool ident;
} effect_handler_context_t;

typedef bool (*effect_handler_f)(effect_handler_context_t *);

/**
 * Structure for atomic effects
 */
struct effect_kind {
	u16b index;          /* Effect index */
	bool aim;            /* Whether the effect requires aiming */
	effect_handler_f handler;
	const char *desc;    /* Effect description */
};


/**
 * Element info - can be improved NRM
 */
static struct breath_info {
	int msgt;            /* Element message type */
	const char *desc;    /* Element description */
} elements[] = {
	{ MSG_BR_ACID, "acid" },
	{ MSG_BR_ELEC, "lightning" },
	{ MSG_BR_FIRE, "fire" },
	{ MSG_BR_FROST, "frost" },
	{ MSG_BR_GAS, "poison gas" },
	{ MSG_BR_LIGHT, "light" },
	{ MSG_BR_DARK, "dark" },
	{ MSG_BR_SOUND, "sound" },
	{ MSG_BR_SHARDS, "shards" },
	{ MSG_BR_NEXUS, "nexus" },
	{ MSG_BR_NETHER, "nether" },
	{ MSG_BR_CHAOS, "chaos" },
	{ 0, "" },
	{ 0, "" },
	{ MSG_BR_DISEN, "disenchantment" },
	{ MSG_BR_GRAVITY, "gravity" },
	{ MSG_BR_INERTIA, "inertia" },
	{ MSG_BR_FORCE, "force" },
	{ MSG_BR_TIME, "time" },
	{ MSG_BR_PLASMA, "plasma" },
	{ 0, "" },
	{ MSG_BR_ELEMENTS, "the elements" },
};


/**
 * Array of stat adjectives
 */
static const char *desc_stat_pos[] =
{
	#define STAT(a, b, c, d, e, f, g, h) #f,
	#include "list-stats.h"
	#undef STAT
};


/**
 * Array of stat opposite adjectives
 */
static const char *desc_stat_neg[] =
{
	#define STAT(a, b, c, d, e, f, g, h) #g,
	#include "list-stats.h"
	#undef STAT
};

int effect_calculate_value(effect_handler_context_t *context, bool use_boost)
{
	int final = 0;

	if (context->value.base > 0 ||
		(context->value.dice > 0 && context->value.sides > 0))
		final = context->value.base +
			damroll(context->value.dice, context->value.sides);

	if (use_boost)
		final *= (100 + context->boost) / 100;

	return final;
}


/**
 * Apply the project() function in a direction, or at a target
 */
static bool project_aimed(int typ, int dir, int dam, int flg)
{
	int py = player->py;
	int px = player->px;

	s16b ty, tx;

	/* Pass through the target if needed */
	flg |= (PROJECT_THRU);

	/* Use the adjacent grid in the given direction as target */
	ty = py + ddy[dir];
	tx = px + ddx[dir];

	/* Ask for a target if no direction given */
	if ((dir == 5) && target_okay())
		target_get(&tx, &ty);

	/* Aim at the target, do NOT explode */
	return (project(-1, 0, ty, tx, dam, typ, flg, 0, 0));
}

/**
 * Apply the project() function to grids the player is touching
 */
static bool project_touch(int dam, int typ, bool aware)
{
	int py = player->py;
	int px = player->px;

	int flg = PROJECT_KILL | PROJECT_HIDE;
	if (aware) flg |= PROJECT_AWARE;
	return (project(-1, 1, py, px, dam, typ, flg, 0, 0));
}

/**
 * Dummy effect, to tell the effect code to pick one of the next 
 * context->value.base effects at random.
 */
bool effect_handler_ATOMIC_RANDOM(effect_handler_context_t *context)
{
	return TRUE;
}

/**
 * Heal the player by a given percentage of their wounds, or a minimum
 * amount, whichever is larger.
 *
 * context->value.base should be the minimum, and
 * context->value.m_bonus the percentage
 */
bool effect_handler_ATOMIC_HEAL_HP(effect_handler_context_t *context)
{
	int num;

	/* Paranoia */
	if ((context->value.m_bonus <= 0) && (context->value.base <= 0))
		return (TRUE);

	/* No healing needed */
	if (player->chp >= player->mhp) return (TRUE);

	/* Figure percentage healing level */
	num = ((player->mhp - player->chp) * context->value.m_bonus) / 100;

	/* Enforce minimum */
	if (num < context->value.base) num = context->value.base;

	/* Gain hitpoints */
	player->chp += num;

	/* Enforce maximum */
	if (player->chp >= player->mhp) {
		player->chp = player->mhp;
		player->chp_frac = 0;
	}

	/* Redraw */
	player->upkeep->redraw |= (PR_HP);

	/* Print a nice message */
	if (num < 5)
		msg("You feel a little better.");
	else if (num < 15)
		msg("You feel better.");
	else if (num < 35)
		msg("You feel much better.");
	else
		msg("You feel very good.");

	/* Notice */
	context->ident = TRUE;

	return (TRUE);
}


/**
 * Feed the player.
 */
bool effect_handler_ATOMIC_NOURISH(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, FALSE);
	player_set_food(player, player->food + amount);

	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_ATOMIC_CRUNCH(effect_handler_context_t *context)
{
	if (one_in_(2))
		msg("It's crunchy.");
	else
		msg("It nearly breaks your tooth!");
	context->ident = TRUE;
	return TRUE;
}

/**
 * Cure a player status condition.
 */
bool effect_handler_ATOMIC_CURE(effect_handler_context_t *context)
{
	int type = context->p1;
	if (player_clear_timed(player, type, TRUE))
		context->ident = TRUE;
	return TRUE;
}

/**
 * Set a (positive or negative) player status condition.
 */
bool effect_handler_ATOMIC_TIMED_SET(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, FALSE);
	player_set_timed(player, context->p1, amount, TRUE);
	context->ident = TRUE;
	return TRUE;

}

/**
 * Extend a (positive or negative) player status condition.
 * If context->p2 is set, increase by that amount if the status exists already
 */
bool effect_handler_ATOMIC_TIMED_INC(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, FALSE);
	if (!player->timed[context->p1] || !context->p2)
		player_inc_timed(player, context->p1, amount, TRUE, TRUE);
	else
		player_inc_timed(player, context->p1, context->p2, TRUE, TRUE);
	context->ident = TRUE;
	return TRUE;

}

/**
 * Reduce a (positive or negative) player status condition.
 */
bool effect_handler_ATOMIC_TIMED_DEC(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, FALSE);
	player_dec_timed(player, context->p1, amount, TRUE);
	context->ident = TRUE;
	return TRUE;

}

/**
 * Make the player, um, lose food.
 */
bool effect_handler_ATOMIC_SET_NOURISH(effect_handler_context_t *context)
{
	player_set_food(player, context->p1);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_ATOMIC_CONFUSING(effect_handler_context_t *context)
{
	if (player->confusing == 0) {
		msg("Your hands begin to glow.");
		player->confusing = TRUE;
		context->ident = TRUE;
	}
	return TRUE;
}

/**
 * Create a "glyph of warding".
 */
bool effect_handler_ATOMIC_RUNE(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;

	/* Always notice */
	context->ident = TRUE;

	/* See if the effect works */
	if (!square_canward(cave, py, px)) {
		msg("There is no clear floor on which to cast the spell.");
		return TRUE;
	}

	/* Create a glyph */
	square_add_ward(cave, py, px);

	/* Push objects off the grid */
	if (square_object(cave, py, px))
		push_object(py, px);

	return (TRUE);
}

/**
 * Restore a stat.  The stat index is context->p1.
 */
bool effect_handler_ATOMIC_RESTORE_STAT(effect_handler_context_t *context)
{
	int stat = context->p1;

	/* Check bounds */
	if (stat < 0 || stat >= STAT_MAX) return FALSE;

	/* Not needed */
	if (player->stat_cur[stat] == player->stat_max[stat])
		return TRUE;

	/* Restore */
	player->stat_cur[stat] = player->stat_max[stat];

	/* Recalculate bonuses */
	player->upkeep->update |= (PU_BONUS);
	update_stuff(player->upkeep);

	/* Message */
	msg("You feel less %s.", desc_stat_neg[stat]);

	/* Success */
	context->ident = TRUE;

	return (TRUE);
}

/**
 * Drain a stat temporarily.  The stat index is context->p1.
 */
bool effect_handler_ATOMIC_DRAIN_STAT(effect_handler_context_t *context)
{
	int stat = context->p1;
	int flag = sustain_flag(stat);

	/* Bounds check */
	if (flag < 0) return FALSE;

	/* Sustain */
	if (player_of_has(player, flag)) {
		/* Notice effect */
		wieldeds_notice_flag(player, flag);

		/* Message */
		msg("You feel very %s for a moment, but the feeling passes.",
		           desc_stat_neg[stat]);

		/* Notice */
		context->ident = TRUE;

		return (TRUE);
	}

	/* Attempt to reduce the stat */
	if (player_stat_dec(player, stat, FALSE)){
		int dam = effect_calculate_value(context, FALSE);

		/* Message */
		msgt(MSG_DRAIN_STAT, "You feel very %s.", desc_stat_neg[stat]);
		take_hit(player, dam, "stat drain");

		/* Notice */
		context->ident = TRUE;
	}

	return (TRUE);
}

/**
 * Lose a stat point permanently, in a stat other than the one specified
 * in context->p1.
 */
bool effect_handler_ATOMIC_LOSE_RANDOM_STAT(effect_handler_context_t *context)
{
	int safe_stat = context->p1;
	int loss_stat = randint0(STAT_MAX - 1);

	/* Skip the safe stat */
	if (loss_stat == safe_stat) loss_stat++;

	/* Attempt to reduce the stat */
	if (player_stat_dec(player, loss_stat, TRUE)) {
		/* Notice */
		context->ident = TRUE;

		/* Message */
		msgt(MSG_DRAIN_STAT, "You feel very %s.", desc_stat_neg[loss_stat]);
	}

	return (TRUE);
}


/**
 * Gain a stat point.  The stat index is context->p1.
 */
bool effect_handler_ATOMIC_GAIN_STAT(effect_handler_context_t *context)
{
	int stat = context->p1;

	/* Attempt to increase */
	if (player_stat_inc(player, stat)) {
		/* Message */
		msg("You feel very %s!", desc_stat_pos[stat]);

		/* Notice */
		context->ident = TRUE;
	}

	return (TRUE);
}

/**
 * Restores any drained experience
 */
bool effect_handler_ATOMIC_RESTORE_EXP(effect_handler_context_t *context)
{
	/* Restore experience */
	if (player->exp < player->max_exp) {
		/* Message */
		msg("You feel your life energies returning.");
		player_exp_gain(player, player->max_exp - player->exp);

        /* Recalculate max. hitpoints */
        update_stuff(player->upkeep);

		/* Did something */
		context->ident = TRUE;
	}

	return (TRUE);
}

bool effect_handler_ATOMIC_GAIN_EXP(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, FALSE);
	if (player->exp < PY_MAX_EXP) {
		msg("You feel more experienced.");
		player_exp_gain(player, amount);
		context->ident = TRUE;
	}
	return TRUE;
}

bool effect_handler_ATOMIC_LOSE_EXP(effect_handler_context_t *context)
{
	if (!player_of_has(player, OF_HOLD_LIFE) && (player->exp > 0)) {
		msg("You feel your memories fade.");
		player_exp_lose(player, player->exp / 4, FALSE);
	}
	context->ident = TRUE;
	wieldeds_notice_flag(player, OF_HOLD_LIFE);
	return TRUE;
}

bool effect_handler_ATOMIC_RESTORE_MANA(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, FALSE);
	if (!amount) amount = player->msp;
	if (player->csp < player->msp) {
		player->csp += amount;
		if (player->csp > player->msp) {
			player->csp = player->msp;
			player->csp_frac = 0;
			msg("You feel your head clear.");
		} else
			msg("You feel your head clear somewhat.");
		player->upkeep->redraw |= (PR_MANA);
		context->ident = TRUE;
	}
	return TRUE;
}

/**
 * Will need revamping with curses - NRM
 */
bool effect_handler_ATOMIC_REMOVE_CURSE(effect_handler_context_t *context)
{
	if (remove_curse())
	{
		if (!player->timed[TMD_BLIND])
			msg("The air around your body glows blue for a moment...");
		else
			msg("You feel as if someone is watching over you.");

		context->ident = TRUE;
	}
	return TRUE;
}

/**
 * Will need revamping with curses - NRM
 */
bool effect_handler_ATOMIC_REMOVE_ALL_CURSE(effect_handler_context_t *context)
{
	remove_all_curse();
	context->ident = TRUE;
	return TRUE;
}

/**
 * Set word of recall as appropriate - set_recall() probably needs work NRM
 */
bool effect_handler_ATOMIC_RECALL(effect_handler_context_t *context)
{
	context->ident = TRUE;
	(void) set_recall();
	return TRUE;
}

bool effect_handler_ATOMIC_DEEP_DESCENT(effect_handler_context_t *context)
{
	int i, target_depth = player->max_depth;

	/* Calculate target depth */
	for (i = 5; i > 0; i--) {
		if (is_quest(target_depth)) break;
		if (target_depth >= MAX_DEPTH - 1) break;

		target_depth++;
	}

	if (target_depth > player->depth) {
		msgt(MSG_TPLEVEL, "The air around you starts to swirl...");
		player->deep_descent = 3 + randint1(4);
		context->ident = TRUE;
		return TRUE;
	} else {
		msgt(MSG_TPLEVEL, "You sense a malevolent presence blocking passage to the levels below.");
		context->ident = TRUE;
		return FALSE;
	}
}

/**
 * Map an area around the player.  The height to map above and below the player
 * is context->value.dice, the width either side of the player context->value.sides.
 *
 */
bool effect_handler_ATOMIC_MAP_AREA(effect_handler_context_t *context)
{
	int i, x, y;
	int x1, x2, y1, y2;
	int y_dist = context->value.dice;
	int x_dist = context->value.sides;

	/* Pick an area to map */
	y1 = player->py - y_dist;
	y2 = player->py + y_dist;
	x1 = player->px - x_dist;
	x2 = player->px + x_dist;

	/* Drag the co-ordinates into the dungeon */
	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > cave->height - 1) y2 = cave->height - 1;
	if (x2 > cave->width - 1) x2 = cave->width - 1;

	/* Scan the dungeon */
	for (y = y1; y < y2; y++) {
		for (x = x1; x < x2; x++) {
			/* Some squares can't be mapped */
			if (square_is_no_map(cave, y, x)) continue;

			/* All non-walls are "checked" */
			if (!square_seemslikewall(cave, y, x)) {
				if (!square_in_bounds_fully(cave, y, x)) continue;

				/* Memorize normal features */
				if (square_isinteresting(cave, y, x)) {
					/* Memorize the object */
					sqinfo_on(cave->info[y][x], SQUARE_MARK);
					square_light_spot(cave, y, x);
				}

				/* Memorize known walls */
				for (i = 0; i < 8; i++) {
					int yy = y + ddy_ddd[i];
					int xx = x + ddx_ddd[i];

					/* Memorize walls (etc) */
					if (square_seemslikewall(cave, yy, xx)) {
						/* Memorize the walls */
						sqinfo_on(cave->info[yy][xx], SQUARE_MARK);
						square_light_spot(cave, yy, xx);
					}
				}
			}
		}
	}
	/* Notice */
	context->ident = TRUE;

	return TRUE;
}

/**
 * Detect traps around the player.  The height to detect above and below the
 * player is context->value.dice, the width either side of the player context->value.sides.
 */
bool effect_handler_ATOMIC_DETECT_TRAPS(effect_handler_context_t *context)
{
	int x, y;
	int x1, x2, y1, y2;
	int y_dist = context->value.dice;
	int x_dist = context->value.sides;

	bool detect = FALSE;

	object_type *o_ptr;

	/* Pick an area to detect */
	y1 = player->py - y_dist;
	y2 = player->py + y_dist;
	x1 = player->px - x_dist;
	x2 = player->px + x_dist;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > cave->height - 1) y2 = cave->height - 1;
	if (x2 > cave->width - 1) x2 = cave->width - 1;


	/* Scan the dungeon */
	for (y = y1; y < y2; y++) {
		for (x = x1; x < x2; x++) {
			if (!square_in_bounds_fully(cave, y, x)) continue;

			/* Detect traps */
			if (square_player_trap(cave, y, x))
				/* Reveal trap */
				if (square_reveal_trap(cave, y, x, 100, FALSE))
					detect = TRUE;

			/* Scan all objects in the grid to look for traps on chests */
			for (o_ptr = get_first_object(y, x); o_ptr;
				 o_ptr = get_next_object(o_ptr)) {
				/* Skip anything not a trapped chest */
				if (!is_trapped_chest(o_ptr)) continue;

				/* Identify once */
				if (!object_is_known(o_ptr)) {
					/* Know the trap */
					object_notice_everything(o_ptr);

					/* Notice it */
					disturb(player, 0);

					/* We found something to detect */
					detect = TRUE;
				}
			}

			/* Mark as trap-detected */
			sqinfo_on(cave->info[y][x], SQUARE_DTRAP);
		}
	}

	/* Rescan the map for the new dtrap edge */
	for (y = y1 - 1; y < y2 + 1; y++) {
		for (x = x1 - 1; x < x2 + 1; x++) {
			if (!square_in_bounds_fully(cave, y, x)) continue;

			/* See if this grid is on the edge */
			if (dtrap_edge(y, x)) {
				sqinfo_on(cave->info[y][x], SQUARE_DEDGE);
			} else {
				sqinfo_off(cave->info[y][x], SQUARE_DEDGE);
			}

			/* Redraw */
			square_light_spot(cave, y, x);
		}
	}

	/* Describe */
	if (detect)
		msg("You sense the presence of traps!");

	/* Trap detection always makes you aware, even if no traps are present */
	else
		msg("You sense no traps.");

	/* Mark the redraw flag */
	player->upkeep->redraw |= (PR_DTRAP);

	/* Notice */
	context->ident = TRUE;

	return TRUE;
}

/**
 * Detect doors around the player.  The height to detect above and below the
 * player is context->value.dice, the width either side of the player context->value.sides.
 */
bool effect_handler_ATOMIC_DETECT_DOORS(effect_handler_context_t *context)
{
	int x, y;
	int x1, x2, y1, y2;
	int y_dist = context->value.dice;
	int x_dist = context->value.sides;

	bool doors = FALSE;

	/* Pick an area to detect */
	y1 = player->py - y_dist;
	y2 = player->py + y_dist;
	x1 = player->px - x_dist;
	x2 = player->px + x_dist;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > cave->height - 1) y2 = cave->height - 1;
	if (x2 > cave->width - 1) x2 = cave->width - 1;

	/* Scan the dungeon */
	for (y = y1; y < y2; y++) {
		for (x = x1; x < x2; x++) {
			if (!square_in_bounds_fully(cave, y, x)) continue;

			/* Detect secret doors - improve later NRM */
			if (square_issecretdoor(cave, y, x))
				place_closed_door(cave, y, x);

			/* Detect doors */
			if (square_isdoor(cave, y, x)) {
				/* Hack -- Memorize */
				sqinfo_on(cave->info[y][x], SQUARE_MARK);

				/* Redraw */
				square_light_spot(cave, y, x);

				/* Obvious */
				doors = TRUE;
				context->ident = TRUE;
			}
		}
	}

	/* Describe */
	if (doors)
		msg("You sense the presence of doors!");
	else if (context->aware)
		msg("You sense no doors.");

	return TRUE;
}

/**
 * Detect stairs around the player.  The height to detect above and below the
 * player is context->value.dice, the width either side of the player context->value.sides.
 */
bool effect_handler_ATOMIC_DETECT_STAIRS(effect_handler_context_t *context)
{
	int x, y;
	int x1, x2, y1, y2;
	int y_dist = context->value.dice;
	int x_dist = context->value.sides;

	bool stairs = FALSE;

	/* Pick an area to detect */
	y1 = player->py - y_dist;
	y2 = player->py + y_dist;
	x1 = player->px - x_dist;
	x2 = player->px + x_dist;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > cave->height - 1) y2 = cave->height - 1;
	if (x2 > cave->width - 1) x2 = cave->width - 1;

	/* Scan the dungeon */
	for (y = y1; y < y2; y++) {
		for (x = x1; x < x2; x++) {
			if (!square_in_bounds_fully(cave, y, x)) continue;

			/* Detect stairs */
			if (square_isstairs(cave, y, x)) {
				/* Hack -- Memorize */
				sqinfo_on(cave->info[y][x], SQUARE_MARK);

				/* Redraw */
				square_light_spot(cave, y, x);

				/* Obvious */
				stairs = TRUE;
				context->ident = TRUE;
			}
		}
	}

	/* Describe */
	if (stairs)
		msg("You sense the presence of stairs!");
	else if (context->aware)
		msg("You sense no stairs.");

	return TRUE;
}


/**
 * Detect buried gold around the player.  The height to detect above and below
 * the player is context->value.dice, the width either side of the player context->value.sides, 
 * and setting context->boost to -1 suppresses messages.
 */
bool effect_handler_ATOMIC_DETECT_GOLD(effect_handler_context_t *context)
{
	int x, y;
	int x1, x2, y1, y2;
	int y_dist = context->value.dice;
	int x_dist = context->value.sides;

	bool gold_buried = FALSE;

	/* Pick an area to detect */
	y1 = player->py - y_dist;
	y2 = player->py + y_dist;
	x1 = player->px - x_dist;
	x2 = player->px + x_dist;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > cave->height - 1) y2 = cave->height - 1;
	if (x2 > cave->width - 1) x2 = cave->width - 1;

	/* Scan the dungeon */
	for (y = y1; y < y2; y++) {
		for (x = x1; x < x2; x++) {
			if (!square_in_bounds_fully(cave, y, x)) continue;

			square_show_vein(cave, y, x);

			/* Magma/Quartz + Known Gold */
			if (square_hasgoldvein(cave, y, x)) {
				/* Hack -- Memorize */
				sqinfo_on(cave->info[y][x], SQUARE_MARK);

				/* Redraw */
				square_light_spot(cave, y, x);

				/* Detect */
				gold_buried = TRUE;
				context->ident = TRUE;
			}
		}
	}

	/* A bit of a hack to allow use in silent gold detection - NRM */
	if (context->boost != -1) {
		if (gold_buried)
			msg("You sense the presence of buried treasure!");
		else if (context->aware)
			msg("You sense no treasure.");
	}

	return TRUE;
}

/**
 * Sense objects around the player.  The height to sense above and below the
 * player is context->value.dice, the width either side of the player
 * context->value.sides
 */
bool effect_handler_ATOMIC_SENSE_OBJECTS(effect_handler_context_t *context)
{
	int i, x, y;
	int x1, x2, y1, y2;
	int y_dist = context->value.dice;
	int x_dist = context->value.sides;

	bool objects = FALSE;

	/* Pick an area to sense */
	y1 = player->py - y_dist;
	y2 = player->py + y_dist;
	x1 = player->px - x_dist;
	x2 = player->px + x_dist;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > cave->height - 1) y2 = cave->height - 1;
	if (x2 > cave->width - 1) x2 = cave->width - 1;

	/* Scan objects */
	for (i = 1; i < cave_object_max(cave); i++)	{
		object_type *o_ptr = cave_object(cave, i);

		/* Skip dead objects */
		if (!o_ptr->kind) continue;

		/* Skip held objects */
		if (o_ptr->held_m_idx) continue;

		/* Location */
		y = o_ptr->iy;
		x = o_ptr->ix;

		/* Only sense nearby objects */
		if (x < x1 || y < y1 || x > x2 || y > y2) continue;

		/* Memorize it */
		if (o_ptr->marked == MARK_UNAWARE)
			o_ptr->marked = MARK_AWARE;

		/* Redraw */
		square_light_spot(cave, y, x);

		/* Detected */
		objects = TRUE;
		context->ident = TRUE;
	}

	if (objects)
		msg("You sense the presence of objects!");
	else if (context->aware)
		msg("You sense no objects.");

	return TRUE;
}

/**
 * Detect objects around the player.  The height to detect above and below the
 * player is context->value.dice, the width either side of the player
 * context->value.sides
 */
bool effect_handler_ATOMIC_DETECT_OBJECTS(effect_handler_context_t *context)
{
	int i, x, y;
	int x1, x2, y1, y2;
	int y_dist = context->value.dice;
	int x_dist = context->value.sides;

	bool objects = FALSE;

	/* Pick an area to detect */
	y1 = player->py - y_dist;
	y2 = player->py + y_dist;
	x1 = player->px - x_dist;
	x2 = player->px + x_dist;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > cave->height - 1) y2 = cave->height - 1;
	if (x2 > cave->width - 1) x2 = cave->width - 1;

	/* Scan objects */
	for (i = 1; i < cave_object_max(cave); i++)	{
		object_type *o_ptr = cave_object(cave, i);

		/* Skip dead objects */
		if (!o_ptr->kind) continue;

		/* Skip held objects */
		if (o_ptr->held_m_idx) continue;

		/* Location */
		y = o_ptr->iy;
		x = o_ptr->ix;

		/* Only detect nearby objects */
		if (x < x1 || y < y1 || x > x2 || y > y2) continue;

		/* Memorize it */
		o_ptr->marked = MARK_SEEN;

		/* Redraw */
		square_light_spot(cave, y, x);

		/* Detect */
		if (!ignore_item_ok(o_ptr)) {
			objects = TRUE;
			context->ident = TRUE;
		}
	}

	if (objects)
		msg("You detect the presence of objects!");
	else if (context->aware)
		msg("You detect no objects.");

	return TRUE;
}

/**
 * Detect visible monsters around the player.  The height to detect above and
 * below the player is context->value.dice, the width either side of the player
 * context->value.sides.
 */
bool effect_handler_ATOMIC_DETECT_VISIBLE_MONSTERS(effect_handler_context_t *context)
{
	int i, x, y;
	int x1, x2, y1, y2;
	int y_dist = context->value.dice;
	int x_dist = context->value.sides;

	bool monsters = FALSE;

	/* Pick an area to detect */
	y1 = player->py - y_dist;
	y2 = player->py + y_dist;
	x1 = player->px - x_dist;
	x2 = player->px + x_dist;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > cave->height - 1) y2 = cave->height - 1;
	if (x2 > cave->width - 1) x2 = cave->width - 1;

	/* Scan monsters */
	for (i = 1; i < cave_monster_max(cave); i++) {
		monster_type *m_ptr = cave_monster(cave, i);

		/* Skip dead monsters */
		if (!m_ptr->race) continue;

		/* Location */
		y = m_ptr->fy;
		x = m_ptr->fx;

		/* Only detect nearby monsters */
		if (x < x1 || y < y1 || x > x2 || y > y2) continue;

		/* Detect all non-invisible, obvious monsters */
		if (!rf_has(m_ptr->race->flags, RF_INVISIBLE) && !m_ptr->unaware) {
			/* Hack -- Detect the monster */
			m_ptr->mflag |= (MFLAG_MARK | MFLAG_SHOW);

			/* Update monster recall window */
			if (player->upkeep->monster_race == m_ptr->race)
				/* Redraw stuff */
				player->upkeep->redraw |= (PR_MONSTER);

			/* Update the monster */
			update_mon(m_ptr, FALSE);

			/* Detect */
			monsters = TRUE;
			context->ident = TRUE;
		}
	}

	if (monsters)
		msg("You sense the presence of monsters!");
	else if (context->aware)
		msg("You sense no monsters.");

	return TRUE;
}


/**
 * Detect invisible monsters around the player.  The height to detect above and
 * below the player is context->value.dice, the width either side of the player
 * context->value.sides.
 */
bool effect_handler_ATOMIC_DETECT_INVISIBLE_MONSTERS(effect_handler_context_t *context)
{
	int i, x, y;
	int x1, x2, y1, y2;
	int y_dist = context->value.dice;
	int x_dist = context->value.sides;

	bool monsters = FALSE;

	/* Pick an area to detect */
	y1 = player->py - y_dist;
	y2 = player->py + y_dist;
	x1 = player->px - x_dist;
	x2 = player->px + x_dist;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > cave->height - 1) y2 = cave->height - 1;
	if (x2 > cave->width - 1) x2 = cave->width - 1;

	/* Scan monsters */
	for (i = 1; i < cave_monster_max(cave); i++) {
		monster_type *m_ptr = cave_monster(cave, i);
		monster_lore *l_ptr;

		/* Skip dead monsters */
		if (!m_ptr->race) continue;

		l_ptr = get_lore(m_ptr->race);

		/* Location */
		y = m_ptr->fy;
		x = m_ptr->fx;

		/* Only detect nearby monsters */
		if (x < x1 || y < y1 || x > x2 || y > y2) continue;

		/* Detect invisible monsters */
		if (rf_has(m_ptr->race->flags, RF_INVISIBLE)) {
			/* Take note that they are invisible */
			rf_on(l_ptr->flags, RF_INVISIBLE);

			/* Update monster recall window */
			if (player->upkeep->monster_race == m_ptr->race)
				player->upkeep->redraw |= (PR_MONSTER);

			/* Detect the monster */
			m_ptr->mflag |= (MFLAG_MARK | MFLAG_SHOW);

			/* Update the monster */
			update_mon(m_ptr, FALSE);

			/* Detect */
			monsters = TRUE;
			context->ident = TRUE;
		}
	}

	if (monsters)
		msg("You sense the presence of invisible creatures!");
	else if (context->aware)
		msg("You sense no invisible creatures.");

	return TRUE;
}



/**
 * Detect evil monsters around the player.  The height to detect above and
 * below the player is context->value.dice, the width either side of the player
 * context->value.sides.
 */
bool effect_handler_ATOMIC_DETECT_EVIL(effect_handler_context_t *context)
{
	int i, x, y;
	int x1, x2, y1, y2;
	int y_dist = context->value.dice;
	int x_dist = context->value.sides;

	bool monsters = FALSE;

	/* Pick an area to detect */
	y1 = player->py - y_dist;
	y2 = player->py + y_dist;
	x1 = player->px - x_dist;
	x2 = player->px + x_dist;

	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;
	if (y2 > cave->height - 1) y2 = cave->height - 1;
	if (x2 > cave->width - 1) x2 = cave->width - 1;

	/* Scan monsters */
	for (i = 1; i < cave_monster_max(cave); i++) {
		monster_type *m_ptr = cave_monster(cave, i);
		monster_lore *l_ptr;

		/* Skip dead monsters */
		if (!m_ptr->race) continue;

		l_ptr = get_lore(m_ptr->race);

		/* Location */
		y = m_ptr->fy;
		x = m_ptr->fx;

		/* Only detect nearby monsters */
		if (x < x1 || y < y1 || x > x2 || y > y2) continue;

		/* Detect evil monsters */
		if (rf_has(m_ptr->race->flags, RF_EVIL)) {
			/* Take note that they are evil */
			rf_on(l_ptr->flags, RF_EVIL);

			/* Update monster recall window */
			if (player->upkeep->monster_race == m_ptr->race)
				player->upkeep->redraw |= (PR_MONSTER);

			/* Detect the monster */
			m_ptr->mflag |= (MFLAG_MARK | MFLAG_SHOW);

			/* Update the monster */
			update_mon(m_ptr, FALSE);

			/* Detect */
			monsters = TRUE;
			context->ident = TRUE;
		}
	}

	if (monsters)
		msg("You sense the presence of evil creatures!");
	else if (context->aware)
		msg("You sense no evil creatures.");

	return TRUE;
}

/**
 * Create stairs at the player location
 */
bool effect_handler_ATOMIC_CREATE_STAIRS(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;

	context->ident = TRUE;

	/* Only allow stairs to be created on empty floor */
	if (!square_isfloor(cave, py, px))
	{
		msg("There is no empty floor here.");
		return TRUE;
	}

	/* Push objects off the grid */
	if (cave->o_idx[py][px]) push_object(py, px);

	square_add_stairs(cave, py, px, player->depth);

	return TRUE;
}

/**
 * Apply disenchantment to the player's stuff.
 */
bool effect_handler_ATOMIC_DISENCHANT(effect_handler_context_t *context)
{
	int i, count = 0;
	object_type *o_ptr;
	char o_name[80];

	/* Count slots */
	for (i = 0; i < player->body.count; i++) {
		/* Ignore rings, amulets and lights */
		if (slot_type_is(i, EQUIP_RING)) continue;
		if (slot_type_is(i, EQUIP_AMULET)) continue;
		if (slot_type_is(i, EQUIP_LIGHT)) continue;

		/* Count disenchantable slots */
		count++;
	}

	/* Pick one at random */
	for (i = player->body.count - 1; i >= 0; i--) {
		/* Ignore rings, amulets and lights */
		if (slot_type_is(i, EQUIP_RING)) continue;
		if (slot_type_is(i, EQUIP_AMULET)) continue;
		if (slot_type_is(i, EQUIP_LIGHT)) continue;

		if (one_in_(count--)) break;
	}

	/* Get the item */
	o_ptr = equipped_item_by_slot(player, i);

	/* No item, nothing happens */
	if (!o_ptr->kind) return TRUE;

	/* Nothing to disenchant */
	if ((o_ptr->to_h <= 0) && (o_ptr->to_d <= 0) && (o_ptr->to_a <= 0))
		return TRUE;

	/* Describe the object */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);

	/* Artifacts have a 60% chance to resist */
	if (o_ptr->artifact && (randint0(100) < 60)) {
		/* Message */
		msg("Your %s (%c) resist%s disenchantment!", o_name, equip_to_label(i),
			((o_ptr->number != 1) ? "" : "s"));

		/* Notice */
		context->ident = TRUE;

		return TRUE;
	}

	/* Apply disenchantment, depending on which kind of equipment */
	if (slot_type_is(i, EQUIP_WEAPON) || slot_type_is(i, EQUIP_BOW)) {
		/* Disenchant to-hit */
		if (o_ptr->to_h > 0) o_ptr->to_h--;
		if ((o_ptr->to_h > 5) && (randint0(100) < 20)) o_ptr->to_h--;

		/* Disenchant to-dam */
		if (o_ptr->to_d > 0) o_ptr->to_d--;
		if ((o_ptr->to_d > 5) && (randint0(100) < 20)) o_ptr->to_d--;
	} else {
		/* Disenchant to-ac */
		if (o_ptr->to_a > 0) o_ptr->to_a--;
		if ((o_ptr->to_a > 5) && (randint0(100) < 20)) o_ptr->to_a--;
	}

	/* Message */
	msg("Your %s (%c) %s disenchanted!", o_name, equip_to_label(i),
		((o_ptr->number != 1) ? "were" : "was"));

	/* Recalculate bonuses */
	player->upkeep->update |= (PU_BONUS);

	/* Window stuff */
	player->upkeep->redraw |= (PR_EQUIP);

	/* Notice */
	context->ident = TRUE;

	return TRUE;
}


/**
 * Enchant an item (in the inventory or on the floor)
 * Note that armour, to hit or to dam is controlled by context->p1
 *
 * Work on incorporating enchant_spell() has been postponed...NRM
 */
bool effect_handler_ATOMIC_ENCHANT(effect_handler_context_t *context)
{
	int value = randcalc(context->value, player->depth, RANDOMISE);
	context->ident = TRUE;

	if (context->p1 & ENCH_TOHIT)
		enchant_spell(value, 0, 0);
	if (context->p1 & ENCH_TODAM)
		enchant_spell(0, value, 0);
	if (context->p1 & ENCH_TOAC)
		enchant_spell(0, 0, value);

	return TRUE;
}

/**
 * Slack - NRM
 */
bool effect_handler_ATOMIC_IDENTIFY(effect_handler_context_t *context)
{
	context->ident = TRUE;
	ident_spell();
	return TRUE;
}

/**
 * Slack - NRM
 */
bool effect_handler_ATOMIC_IDENTIFY_PACK(effect_handler_context_t *context)
{
	context->ident = TRUE;
	identify_pack();
	return TRUE;
}

/*
 * Hook for "get_item()".  Determine if something is rechargable.
 */
static bool item_tester_hook_recharge(const object_type *o_ptr)
{
	/* Recharge staves and wands */
	if (tval_can_have_charges(o_ptr)) return TRUE;

	return FALSE;
}


/**
 * Recharge a wand or staff from the pack or on the floor.  Recharge strength
 * is context->value.base.
 *
 * It is harder to recharge high level, and highly charged wands.
 */
bool effect_handler_ATOMIC_RECHARGE(effect_handler_context_t *context)
{
	int i, t, item, lev;
	int strength = context->value.base;
	object_type *o_ptr;
	const char *q, *s;

	/* Immediately obvious */
	context->ident = TRUE;

	/* Get an item */
	q = "Recharge which item? ";
	s = "You have nothing to recharge.";
	if (!get_item(&item, q, s, 0, item_tester_hook_recharge,
				  (USE_INVEN | USE_FLOOR)))
		return TRUE;

	o_ptr = object_from_item_idx(item);

	/* Extract the object "level" */
	lev = o_ptr->kind->level;

	/*
	 * Chance of failure = 1 time in
	 * [Spell_strength + 100 - item_level - 10 * charge_per_item]/15
	 */
	i = (strength + 100 - lev - (10 * (o_ptr->pval / o_ptr->number))) / 15;

	/* Back-fire */
	if ((i <= 1) || one_in_(i)) {
		msg("The recharge backfires!");
		msg("There is a bright flash of light.");

		/* Reduce the charges of rods/wands/staves */
		reduce_charges(o_ptr, 1);

		/* Reduce and describe inventory */
		if (item >= 0) {
			inven_item_increase(item, -1);
			inven_item_describe(item);
			inven_item_optimize(item);
		} else {
			/* Reduce and describe floor item */
			floor_item_increase(0 - item, -1);
			floor_item_describe(0 - item);
			floor_item_optimize(0 - item);
		}
	} else {
		/* Extract a "power" */
		t = (strength / (lev + 2)) + 1;

		/* Recharge based on the power */
		if (t > 0) o_ptr->pval += 2 + randint1(t);
	}

	/* Update the gear */
	player->upkeep->update |= (PU_INVEN);

	/* Combine the pack (later) */
	player->upkeep->notice |= (PN_COMBINE);

	/* Redraw stuff */
	player->upkeep->redraw |= (PR_INVEN);

	/* Something was done */
	return TRUE;
}

/**
 * Apply a "project()" directly to all viewable monsters.  If context->p2 is
 * set, the effect damage boost is applied.  This is a hack - NRM
 *
 * Note that affected monsters are NOT auto-tracked by this usage.
 */
bool effect_handler_ATOMIC_PROJECT_LOS(effect_handler_context_t *context)
{
	int i, x, y;
	int dam = effect_calculate_value(context, context->p2 ? TRUE : FALSE);
	int typ = context->p1;

	int flg = PROJECT_JUMP | PROJECT_KILL | PROJECT_HIDE;

	if (context->aware) flg |= PROJECT_AWARE;

	/* Affect all (nearby) monsters */
	for (i = 1; i < cave_monster_max(cave); i++) {
		monster_type *m_ptr = cave_monster(cave, i);

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->race) continue;

		/* Location */
		y = m_ptr->fy;
		x = m_ptr->fx;

		/* Require line of sight */
		if (!player_has_los_bold(y, x)) continue;

		/* Jump directly to the target monster */
		if (project(-1, 0, y, x, dam, typ, flg, 0, 0)) context->ident = TRUE;
	}

	/* Result */
	return TRUE;
}

bool effect_handler_ATOMIC_ACQUIRE(effect_handler_context_t *context)
{
	int num = effect_calculate_value(context, FALSE);
	acquirement(player->py, player->px, player->depth, num, TRUE);
	context->ident = TRUE;
	return TRUE;
}

/**
 * Wake up all monsters, and speed up "los" monsters.  The index of the monster
 * doing the aggravating (or 0 for the player) is context->p2.
 *
 * Possibly the los test should be from the aggravating monster, rather than
 * automatically the player - NRM
 */
bool effect_handler_ATOMIC_AGGRAVATE(effect_handler_context_t *context)
{
	int i;
	bool sleep = FALSE;
	monster_type *who = context->p2 ? cave_monster(cave, context->p2) : NULL;

	/* Immediately obvious if the player did it */
	if (!who) {
		msg("There is a high pitched humming noise.");
		context->ident = TRUE;
	}

	/* Aggravate everyone nearby */
	for (i = 1; i < cave_monster_max(cave); i++) {
		monster_type *m_ptr = cave_monster(cave, i);

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->race) continue;

		/* Skip aggravating monster (or player) */
		if (m_ptr == who) continue;

		/* Wake up nearby sleeping monsters */
		if ((m_ptr->cdis < MAX_SIGHT * 2) && m_ptr->m_timed[MON_TMD_SLEEP]) {
			mon_clear_timed(m_ptr, MON_TMD_SLEEP, MON_TMD_FLG_NOMESSAGE, FALSE);
			sleep = TRUE;
			context->ident = TRUE;
		}

		/* Speed up monsters in line of sight */
		if (player_has_los_bold(m_ptr->fy, m_ptr->fx)) {
			mon_inc_timed(m_ptr, MON_TMD_FAST, 25, MON_TMD_FLG_NOTIFY, FALSE);
			context->ident = TRUE;
		}
	}

	/* Messages */
	if (sleep) msg("You hear a sudden stirring in the distance!");

	return TRUE;
}

/**
 * Summon context->value monsters of context->p1 type.
 * Better handling of context->p1 is sorely needed
 */
bool effect_handler_ATOMIC_SUMMON(effect_handler_context_t *context)
{
	int i;
	int num = effect_calculate_value(context, FALSE);
	int type = context->p1 ? context->p1 : 0;
	int msgt = MSG_SUM_MONSTER;

	if (type == S_ANIMAL) msgt = MSG_SUM_ANIMAL;
	else if (type == S_SPIDER) msgt = MSG_SUM_SPIDER;
	else if (type == S_HOUND) msgt = MSG_SUM_HOUND;
	else if (type == S_HYDRA) msgt = MSG_SUM_HYDRA;
	else if (type == S_AINU) msgt = MSG_SUM_AINU;
	else if (type == S_DEMON) msgt = MSG_SUM_DEMON;
	else if (type == S_UNDEAD) msgt = MSG_SUM_UNDEAD;
	else if (type == S_DRAGON) msgt = MSG_SUM_DRAGON;
	else if (type == S_HI_DEMON) msgt = MSG_SUM_HI_DEMON;
	else if (type == S_HI_UNDEAD) msgt = MSG_SUM_HI_UNDEAD;
	else if (type == S_HI_DRAGON) msgt = MSG_SUM_HI_DRAGON;
	else if (type == S_WRAITH) msgt = MSG_SUM_WRAITH;
	else if (type == S_UNIQUE) msgt = MSG_SUM_UNIQUE;

	sound(msgt);

	for (i = 0; i < num; i++) {
		if (summon_specific(player->py, player->px, player->depth, type, 1))
			context->ident = TRUE;
	}
	return TRUE;
}

/**
 * Delete all non-unique monsters of a given "type" from the level
 */
bool effect_handler_ATOMIC_BANISH(effect_handler_context_t *context)
{
	int i;
	unsigned dam = 0;

	struct keypress typ;

	context->ident = TRUE;

	if (!get_com("Choose a monster race (by symbol) to banish: ", &typ))
		return TRUE;

	/* Delete the monsters of that "type" */
	for (i = 1; i < cave_monster_max(cave); i++) {
		monster_type *m_ptr = cave_monster(cave, i);

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->race) continue;

		/* Hack -- Skip Unique Monsters */
		if (rf_has(m_ptr->race->flags, RF_UNIQUE)) continue;

		/* Skip "wrong" monsters */
		if (!char_matches_key(m_ptr->race->d_char, typ.code)) continue;

		/* Delete the monster */
		delete_monster_idx(i);

		/* Take some damage */
		dam += randint1(4);
	}

	/* Hurt the player */
	take_hit(player, dam, "the strain of casting Banishment");

	/* Update monster list window */
	player->upkeep->redraw |= PR_MONLIST;

	/* Success */
	return TRUE;
}

/**
 * Delete all nearby (non-unique) monsters.  The radius of effect is
 * context->p2 if passed, otherwise the player view radius.
 */
bool effect_handler_ATOMIC_MASS_BANISH(effect_handler_context_t *context)
{
	int i;
	int radius = context->p2 ? context->p2 : MAX_SIGHT;
	unsigned dam = 0;

	context->ident = TRUE;

	/* Delete the (nearby) monsters */
	for (i = 1; i < cave_monster_max(cave); i++) {
		monster_type *m_ptr = cave_monster(cave, i);

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->race) continue;

		/* Hack -- Skip unique monsters */
		if (rf_has(m_ptr->race->flags, RF_UNIQUE)) continue;

		/* Skip distant monsters */
		if (m_ptr->cdis > radius) continue;

		/* Delete the monster */
		delete_monster_idx(i);

		/* Take some damage */
		dam += randint1(3);
	}

	/* Hurt the player */
	take_hit(player, dam, "the strain of casting Mass Banishment");

	/* Update monster list window */
	player->upkeep->redraw |= PR_MONLIST;

	return TRUE;
}

/*
 * Probe nearby monsters
 */
bool effect_handler_ATOMIC_PROBE(effect_handler_context_t *context)
{
	int i;

	bool probe = FALSE;

	/* Probe all (nearby) monsters */
	for (i = 1; i < cave_monster_max(cave); i++) {
		monster_type *m_ptr = cave_monster(cave, i);

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->race) continue;

		/* Require line of sight */
		if (!player_has_los_bold(m_ptr->fy, m_ptr->fx)) continue;

		/* Probe visible monsters */
		if (m_ptr->ml)
		{
			char m_name[80];

			/* Start the message */
			if (!probe) msg("Probing...");

			/* Get "the monster" or "something" */
			monster_desc(m_name, sizeof(m_name), m_ptr,
					MDESC_IND_HID | MDESC_CAPITAL);

			/* Describe the monster */
			msg("%s has %d hit points.", m_name, m_ptr->hp);

			/* Learn all of the non-spell, non-treasure flags */
			lore_do_probe(m_ptr);

			/* Probe worked */
			probe = TRUE;
		}
	}

	/* Done */
	if (probe) {
		msg("That's all.");
		context->ident = TRUE;
	}

	return TRUE;
}

/**
 * Teleport the player to a location up to context->value.base grids away.
 *
 * If no such spaces are readily available, the distance may increase.
 * Try very hard to move the player at least a quarter that distance.
 */
bool effect_handler_ATOMIC_TELEPORT(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;
	int dis = context->value.base;
	int d, i, min, y, x;

	bool look = TRUE;

	context->ident = TRUE;

	/* Check for a no teleport grid */
	if (square_is_no_teleport(cave, py, px) && (dis > 10)) {
		msg("Teleportation forbidden!");
		return TRUE;
	}

	/* Initialize */
	y = py;
	x = px;

	/* Minimum distance */
	min = dis / 2;

	/* Look until done */
	while (look) {
		/* Verify max distance */
		if (dis > 200) dis = 200;

		/* Try several locations */
		for (i = 0; i < 500; i++) {
			/* Pick a (possibly illegal) location */
			while (1) {
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

	/* Lots of updates after monster_swap */
	handle_stuff(player->upkeep);

	return TRUE;
}

/**
 * Teleport player to a grid near the given location
 *
 * This function is slightly obsessive about correctness.
 * This function allows teleporting into vaults (!)
 */
bool effect_handler_ATOMIC_TELEPORT_TO(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;

	int ny = GRID_Y(context->p2);
	int nx = GRID_X(context->p2);

	int y, x, dis = 0, ctr = 0;

	/* Initialize */
	y = py;
	x = px;

	context->ident = TRUE;

	/* Find a usable location */
	while (1) {
		/* Pick a nearby legal location */
		while (1) {
			y = rand_spread(ny, dis);
			x = rand_spread(nx, dis);
			if (square_in_bounds_fully(cave, y, x)) break;
		}

		/* Accept "naked" floor grids */
		if (square_isempty(cave, y, x)) break;

		/* Occasionally advance the distance */
		if (++ctr > (4 * dis * dis + 4 * dis + 1)) {
			ctr = 0;
			dis++;
		}
	}

	/* Sound */
	sound(MSG_TELEPORT);

	/* Move player */
	monster_swap(py, px, y, x);

	/* Lots of updates after monster_swap */
	handle_stuff(player->upkeep);

	return TRUE;
}

/**
 * Teleport the player one level up or down (random when legal)
 */
bool effect_handler_ATOMIC_TELEPORT_LEVEL(effect_handler_context_t *context)
{
	bool up = TRUE, down = TRUE;

	context->ident = TRUE;

	/* No going up with force_descend or in the town */
	if (OPT(birth_force_descend) || !player->depth)
		up = FALSE;

	/* No forcing player down to quest levels if they can't leave */
	if (!up && is_quest(player->max_depth + 1))
		down = FALSE;

	/* Can't leave quest levels or go down deeper than the dungeon */
	if (is_quest(player->depth) || (player->depth >= MAX_DEPTH - 1))
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
		dungeon_change_level(player->depth - 1);
	} else if (down) {
		msgt(MSG_TPLEVEL, "You sink through the floor.");

		if (OPT(birth_force_descend))
			dungeon_change_level(player->max_depth + 1);
		else
			dungeon_change_level(player->depth + 1);
	} else {
		msg("Nothing happens.");
	}

	return TRUE;
}

/**
 * The spell of destruction
 *
 * This spell "deletes" monsters (instead of "killing" them).
 *
 * This is always an effect centred on the player; it is similar to the
 * "earthquake" effect.
 */
bool effect_handler_ATOMIC_DESTRUCTION(effect_handler_context_t *context)
{
	int y, x, k, r = context->p2;
	int y1 = player->py;
	int x1 = player->px;

	context->ident = TRUE;

	/* No effect in town */
	if (!player->depth) {
		msg("The ground shakes for a moment.");
		return TRUE;
	}

	/* Big area of affect */
	for (y = (y1 - r); y <= (y1 + r); y++) {
		for (x = (x1 - r); x <= (x1 + r); x++) {
			/* Skip illegal grids */
			if (!square_in_bounds_fully(cave, y, x)) continue;

			/* Extract the distance */
			k = distance(y1, x1, y, x);

			/* Stay in the circle of death */
			if (k > r) continue;

			/* Lose room and vault */
			sqinfo_off(cave->info[y][x], SQUARE_ROOM);
			sqinfo_off(cave->info[y][x], SQUARE_VAULT);

			/* Lose light */
			sqinfo_off(cave->info[y][x], SQUARE_GLOW);
			square_light_spot(cave, y, x);

			/* Deal with player later */
			if ((y == y1) && (x == x1)) continue;

			/* Delete the monster (if any) */
			delete_monster(y, x);

			/* Don't remove stairs */
			if (square_isstairs(cave, y, x)) continue;

			/* Lose knowledge (keeping knowledge of stairs) */
			sqinfo_off(cave->info[y][x], SQUARE_MARK);

			/* Destroy any grid that isn't a permament wall */
			if (!square_isperm(cave, y, x)) {
				/* Delete objects */
				delete_object(y, x);
				square_destroy(cave, y, x);
			}
		}
	}

	/* Message */
	msg("There is a searing blast of light!");

	/* Blind the player */
	wieldeds_notice_element(player, ELEM_LIGHT);
	if (!player_resists(player, ELEM_LIGHT))
		(void)player_inc_timed(player, TMD_BLIND, 10 + randint1(10),TRUE, TRUE);

	/* Fully update the visuals */
	player->upkeep->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

	/* Fully update the flow */
	player->upkeep->update |= (PU_FORGET_FLOW | PU_UPDATE_FLOW);

	/* Redraw monster list */
	player->upkeep->redraw |= (PR_MONLIST | PR_ITEMLIST);

	return TRUE;
}

/**
 * This effect, the destruction effect, and the earthquake funtion should be
 * rationalised
 */
bool effect_handler_ATOMIC_EARTHQUAKE(effect_handler_context_t *context)
{
	earthquake(player->py, player->px, context->p2);
	context->ident = TRUE;
	return TRUE;
}

bool effect_handler_ATOMIC_ENLIGHTENMENT(effect_handler_context_t *context)
{
	bool full = context->value.base ? TRUE : FALSE;
	if (full)
		msg("An image of your surroundings forms in your mind...");
	wiz_light(cave, full);
	context->ident = TRUE;
	return TRUE;
}

/**
 * Call light around the player
 * Affect all monsters in the projection radius (context->p2)
 */
bool effect_handler_ATOMIC_LIGHT_AREA(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;
	int dam = effect_calculate_value(context, FALSE);
	int rad = context->p2;

	int flg = PROJECT_GRID | PROJECT_KILL;

	/* Message */
	if (!player->timed[TMD_BLIND])
		msg("You are surrounded by a white light.");

	/* Hook into the "project()" function */
	(void)project(-1, rad, py, px, dam, GF_LIGHT_WEAK, flg, 0, 0);

	/* Light up the room */
	light_room(py, px, TRUE);

	/* Assume seen */
	context->ident = TRUE;
	return (TRUE);
}


/**
 * Call darkness around the player
 * Affect all monsters in the projection radius (context->p2)
 */
bool effect_handler_ATOMIC_DARKEN_AREA(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;
	int dam = effect_calculate_value(context, FALSE);
	int rad = context->p2;

	int flg = PROJECT_GRID | PROJECT_KILL;

	/* Message */
	if (!player->timed[TMD_BLIND])
		msg("Darkness surrounds you.");

	/* Hook into the "project()" function */
	(void)project(-1, rad, py, px, dam, GF_DARK_WEAK, flg, 0, 0);

	/* Darken the room */
	light_room(py, px, FALSE);

	/* Assume seen */
	context->ident = TRUE;
	return (TRUE);
}

/**
 * Cast a ball spell
 * Stop if we hit a monster, act as a ball
 * Allow target mode to pass over monsters
 * Affect grids, objects, and monsters
 */
bool effect_handler_ATOMIC_BALL(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;
	int dam = effect_calculate_value(context, TRUE);

	s16b ty = py + 99 * ddy[context->dir];
	s16b tx = px + 99 * ddx[context->dir];

	int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	/* Ask for a target if no direction given */
	if ((context->dir == 5) && target_okay()) {
		flg &= ~(PROJECT_STOP);

		target_get(&tx, &ty);
	}

	/* Aim at the target, explode */
	if (project(-1, context->p2, ty, tx, dam, context->p1, flg, 0, 0))
		context->ident = TRUE;

	return TRUE;
}


/**
 * Breathe an element
 * Stop if we hit a monster, act as a ball (for now)
 * Allow target mode to pass over monsters
 * Affect grids, objects, and monsters
 */
bool effect_handler_ATOMIC_BREATH(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;
	int dam = effect_calculate_value(context, TRUE);
	int type = context->p1;

	s16b ty = py + 99 * ddy[context->dir];
	s16b tx = px + 99 * ddx[context->dir];

	int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	msgt(elements[type].msgt, "You breathe %s.", elements[type].desc);

	/* Ask for a target if no direction given */
	if ((context->dir == 5) && target_okay()) {
		flg &= ~(PROJECT_STOP);

		target_get(&tx, &ty);
	}

	/* Aim at the target, explode */
	if (project(-1, context->p2, ty, tx, dam, type, flg, 0, 0))
		context->ident = TRUE;

	return TRUE;
}


/**
 * Cast multiple non-jumping ball spells at the same target.
 *
 * Targets absolute coordinates instead of a specific monster, so that
 * the death of the monster doesn't change the target's location.
 */
bool effect_handler_ATOMIC_SWARM(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;
	int dam = effect_calculate_value(context, TRUE);
	int num = context->value.m_bonus;

	s16b ty = py + 99 * ddy[context->dir];
	s16b tx = px + 99 * ddx[context->dir];

	int flg = PROJECT_THRU | PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	/* Ask for a target if no direction given (early detonation) */
	if ((context->dir == 5) && target_okay())
		target_get(&tx, &ty);

	while (num--)
	{
		/* Aim at the target.  Hurt items on floor. */
		if (project(-1, context->p2, ty, tx, dam, context->p1, flg, 0, 0))
			context->ident = TRUE;
	}

	return TRUE;
}

/**
 * Cast a line spell in every direction
 * Stop if we hit a monster, act as a ball
 * Affect grids, objects, and monsters
 */
bool effect_handler_ATOMIC_STAR(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;
	int dam = effect_calculate_value(context, TRUE);
	int i;

	s16b ty, tx;

	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL;

	for (i = 0; i < 8; i++) {
		/* Use the current direction */
		ty = py + 99 * ddy[i];
		tx = px + 99 * ddx[i];

		/* Aim at the target */
		if (project(-1, 0, ty, tx, dam, context->p1, flg, 0, 0))
			context->ident = TRUE;
	}
	return TRUE;
}


/**
 * Cast a ball spell in every direction
 * Stop if we hit a monster, act as a ball
 * Affect grids, objects, and monsters
 */
bool effect_handler_ATOMIC_STAR_BALL(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;
	int dam = effect_calculate_value(context, TRUE);
	int i;

	s16b ty, tx;

	int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	for (i = 0; i < 8; i++) {
		/* Use the current direction */
		ty = py + 99 * ddy[i];
		tx = px + 99 * ddx[i];

		/* Aim at the target, explode */
		if (project(-1, context->p2, ty, tx, dam, context->p1, flg, 0, 0))
			context->ident = TRUE;
	}
	return TRUE;
}

/**
 * Messy, will change if breaths do - NRM
 */
bool effect_handler_ATOMIC_RAND_BREATH(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;

	s16b ty = py + 99 * ddy[context->dir];
	s16b tx = px + 99 * ddx[context->dir];

	int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	/* Table of random ball effects and their damages */
	const int breath_types[] = {
		GF_ACID, 200,
		GF_ELEC, 160,
		GF_FIRE, 200,
		GF_COLD, 160,
		GF_POIS, 120
	};

	/* Pick a random (type, damage) tuple in the table */
	int which = 2 * randint0(sizeof(breath_types) / (2 * sizeof(int)));

	/* Ask for a target if no direction given */
	if ((context->dir == 5) && target_okay()) {
		flg &= ~(PROJECT_STOP);

		target_get(&tx, &ty);
	}

	/* Aim at the target, explode */
	(void) project(-1, breath_types[which], ty, tx, breath_types[which + 1], 3, flg, 0, 0);

	context->ident = TRUE;
	return TRUE;
}
 
/**
 * Cast a bolt spell
 * Stop if we hit a monster, as a bolt
 * Affect monsters (not grids or objects)
 */
bool effect_handler_ATOMIC_BOLT(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, TRUE);
	int flg = PROJECT_STOP | PROJECT_KILL;
	if (project_aimed(context->p1, context->dir, dam, flg))
		context->ident = TRUE;
	return TRUE;
}

/**
 * Cast a beam spell
 * Pass through monsters, as a beam
 * Affect monsters (not grids or objects)
 */
bool effect_handler_ATOMIC_BEAM(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, TRUE);
	int flg = PROJECT_BEAM | PROJECT_KILL;
	if (project_aimed(context->p1, context->dir, dam, flg))
		context->ident = TRUE;
	return TRUE;
}

/**
 * Cast a bolt spell, or rarely, a beam spell
 */
bool effect_handler_ATOMIC_BOLT_OR_BEAM(effect_handler_context_t *context)
{
	if (randint0(100) < context->beam)
		return effect_handler_ATOMIC_BEAM(context);
	else
		return effect_handler_ATOMIC_BOLT(context);
}

/**
 * Cast a line spell
 * Pass through monsters, as a beam
 * Affect monsters and grids (not objects)
 */
bool effect_handler_ATOMIC_LINE(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, TRUE);
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL;
	if (project_aimed(context->p1, context->dir, dam, flg))
		context->ident = TRUE;
	return TRUE;
}

/**
 * Cast an alter spell
 * Affect objects and grids (not monsters)
 */
bool effect_handler_ATOMIC_ALTER(effect_handler_context_t *context)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM;
	if (project_aimed(context->p1, context->dir, 0, flg))
		context->ident = TRUE;
	return TRUE;
}

/**
 * Cast a bolt spell
 * Stop if we hit a monster, as a bolt
 * Affect monsters (not grids or objects)
 * Notice stuff based on awareness of the effect
 */
bool effect_handler_ATOMIC_BOLT_AWARE(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, TRUE);
	int flg = PROJECT_STOP | PROJECT_KILL;
	if (context->aware) flg |= PROJECT_AWARE;
	if (project_aimed(context->p1, context->dir, dam, flg))
		context->ident = TRUE;
	return TRUE;
}

/**
 * Affect adjacent grids (radius 1 ball attack)
 */
bool effect_handler_ATOMIC_TOUCH(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, TRUE);
	if (project_touch(dam, context->p1, FALSE))
		context->ident = TRUE;
	return TRUE;
}

/**
 * Affect adjacent grids (radius 1 ball attack)
 * Notice stuff based on awareness of the effect
 */
bool effect_handler_ATOMIC_TOUCH_AWARE(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, TRUE);
	if (project_touch(dam, context->p1, context->aware))
		context->ident = TRUE;
	return TRUE;
}

/**
 * Curse the player's armor
 */
bool effect_handler_ATOMIC_CURSE_ARMOR(effect_handler_context_t *context)
{
	object_type *o_ptr;

	char o_name[80];

	context->ident = TRUE;

	/* Curse the body armor */
	o_ptr = equipped_item_by_slot_name(player, "body");

	/* Nothing to curse */
	if (!o_ptr->kind) return (FALSE);

	/* Describe */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_FULL);

	/* Attempt a saving throw for artifacts */
	if (o_ptr->artifact && (randint0(100) < 50))
		/* Cool */
		msg("A %s tries to %s, but your %s resists the effects!",
		           "terrible black aura", "surround your armor", o_name);

	/* not artifact or failed save... */
	else {
		/* Oops */
		msg("A terrible black aura blasts your %s!", o_name);

		/* Take down bonus a wee bit */
		o_ptr->to_a -= randint1(3);

		/* Curse it */
		flags_set(o_ptr->flags, OF_SIZE, OF_LIGHT_CURSE, OF_HEAVY_CURSE, FLAG_END);

		/* Recalculate bonuses */
		player->upkeep->update |= (PU_BONUS);

		/* Recalculate mana */
		player->upkeep->update |= (PU_MANA);

		/* Window stuff */
		player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);
	}

	return (TRUE);
}


/**
 * Curse the player's weapon
 */
bool effect_handler_ATOMIC_CURSE_WEAPON(effect_handler_context_t *context)
{
	object_type *o_ptr;

	char o_name[80];

	context->ident = TRUE;

	/* Curse the weapon */
	o_ptr = equipped_item_by_slot_name(player, "weapon");

	/* Nothing to curse */
	if (!o_ptr->kind) return (FALSE);

	/* Describe */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_FULL);

	/* Attempt a saving throw */
	if (o_ptr->artifact && (randint0(100) < 50))
		/* Cool */
		msg("A %s tries to %s, but your %s resists the effects!",
		           "terrible black aura", "surround your weapon", o_name);

	/* not artifact or failed save... */
	else {
		/* Oops */
		msg("A terrible black aura blasts your %s!", o_name);

		/* Hurt it a bit */
		o_ptr->to_h = 0 - randint1(3);
		o_ptr->to_d = 0 - randint1(3);

		/* Curse it */
		flags_set(o_ptr->flags, OF_SIZE, OF_LIGHT_CURSE, OF_HEAVY_CURSE, FLAG_END);

		/* Recalculate bonuses */
		player->upkeep->update |= (PU_BONUS);

		/* Recalculate mana */
		player->upkeep->update |= (PU_MANA);

		/* Window stuff */
		player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);
	}

	/* Notice */
	return (TRUE);
}


/**
 * Brand the current weapon
 */
bool effect_handler_ATOMIC_BRAND_WEAPON(effect_handler_context_t *context)
{
	object_type *o_ptr = equipped_item_by_slot_name(player, "weapon");

	/* Select the brand */
	const char *brand = one_in_(2) ? "Flame" : "Frost";

	/* Brand the weapon */
	brand_object(o_ptr, brand);

	context->ident = TRUE;
	return TRUE;
}


/*
 * Hook to specify "ammo"
 */
static bool item_tester_hook_ammo(const object_type *o_ptr)
{
	return tval_is_ammo(o_ptr);
}


/**
 * Brand some (non-magical) ammo
 */
bool effect_handler_ATOMIC_BRAND_AMMO(effect_handler_context_t *context)
{
	int item;
	object_type *o_ptr;
	const char *q, *s;

	/* Select the brand */
	const char *brand = one_in_(3) ? "Flame" : (one_in_(2) ? "Frost" : "Venom");

	context->ident = TRUE;

	/* Get an item */
	q = "Brand which kind of ammunition? ";
	s = "You have nothing to brand.";
	if (!get_item(&item, q, s, 0, item_tester_hook_ammo, (USE_INVEN | USE_QUIVER | USE_FLOOR))) return TRUE;

	o_ptr = object_from_item_idx(item);

	/* Brand the ammo */
	brand_object(o_ptr, brand);

	/* Done */
	return (TRUE);
}

static bool item_tester_hook_bolt(const struct object *o)
{
	return o->tval == TV_BOLT;
}

/**
 * Enchant some (non-magical) bolts
 */
bool effect_handler_ATOMIC_BRAND_BOLTS(effect_handler_context_t *context)
{
	int item;
	object_type *o_ptr;
	const char *q, *s;

	context->ident = TRUE;

	/* Get an item */
	q = "Brand which bolts? ";
	s = "You have no bolts to brand.";
	if (!get_item(&item, q, s, 0, item_tester_hook_bolt, (USE_INVEN | USE_QUIVER | USE_FLOOR))) return TRUE;

	o_ptr = object_from_item_idx(item);

	/* Brand the bolts */
	brand_object(o_ptr, "Flame");

	/* Done */
	return (TRUE);
}


/**
 * Slack - NRM
 */
bool effect_handler_ATOMIC_BIZARRE(effect_handler_context_t *context)
{
	context->ident = TRUE;
	ring_of_power(context->dir);
	return TRUE;
}

/**
 * Super slack - NRM
 */
bool effect_handler_ATOMIC_WONDER(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, FALSE);
	context->ident = TRUE;
	effect_wonder(context->dir, amount, context->beam);
	return TRUE;
}

/*
 * The "wonder" effect.
 *
 * Returns TRUE if the effect is evident.
 */
bool effect_wonder(int dir, int die, int beam)
{
/* This spell should become more useful (more
   controlled) as the player gains experience levels.
   Thus, add 1/5 of the player's level to the die roll.
   This eliminates the worst effects later on, while
   keeping the results quite random.  It also allows
   some potent effects only at high level. */

	bool visible = FALSE;
	int py = player->py;
	int px = player->px;
	int plev = player->lev;

	if (die > 100)
	{
		/* above 100 the effect is always visible */
		msg("You feel a surge of power!");
		visible = TRUE;
	}

	if (die < 8) visible = clone_monster(dir);
	else if (die < 14) visible = speed_monster(dir);
	else if (die < 26) visible = heal_monster(dir);
	else if (die < 31) visible = poly_monster(dir);
	else if (die < 36)
		visible = fire_bolt_or_beam(beam - 10, GF_MISSILE, dir,
		                            damroll(3 + ((plev - 1) / 5), 4));
	else if (die < 41) visible = confuse_monster(dir, plev, FALSE);
	else if (die < 46) visible = fire_ball(GF_POIS, dir, 20 + (plev / 2), 3);
	else if (die < 51) visible = light_line(dir);
	else if (die < 56)
		visible = fire_beam(GF_ELEC, dir, damroll(3+((plev-5)/6), 6));
	else if (die < 61)
		visible = fire_bolt_or_beam(beam-10, GF_COLD, dir,
		                            damroll(5+((plev-5)/4), 8));
	else if (die < 66)
		visible = fire_bolt_or_beam(beam, GF_ACID, dir,
		                            damroll(6+((plev-5)/4), 8));
	else if (die < 71)
		visible = fire_bolt_or_beam(beam, GF_FIRE, dir,
		                            damroll(8+((plev-5)/4), 8));
	else if (die < 76) visible = drain_life(dir, 75);
	else if (die < 81) visible = fire_ball(GF_ELEC, dir, 30 + plev / 2, 2);
	else if (die < 86) visible = fire_ball(GF_ACID, dir, 40 + plev, 2);
	else if (die < 91) visible = fire_ball(GF_ICE, dir, 70 + plev, 3);
	else if (die < 96) visible = fire_ball(GF_FIRE, dir, 80 + plev, 3);
	/* above 100 'visible' is already true */
	else if (die < 101) drain_life(dir, 100 + plev);
	else if (die < 104) earthquake(py, px, 12);
	else if (die < 106) destroy_area(py, px, 15, TRUE);
	else if (die < 108) banishment();
	else if (die < 110) dispel_monsters(120);
	else /* RARE */
	{
		dispel_monsters(150);
		slow_monsters();
		sleep_monsters(TRUE);
		hp_player(300);
	}

	return visible;
}





bool effect_handler_TRAP_DOOR(effect_handler_context_t *context)
{
	msg("You fall through a trap door!");
	if (player_of_has(player, OF_FEATHER)) {
		msg("You float gently down to the next level.");
	} else {
		take_hit(player, damroll(2, 8), "a trap");
	}
	wieldeds_notice_flag(player, OF_FEATHER);

	dungeon_change_level(player->depth + 1);
	return TRUE;
}

bool effect_handler_TRAP_PIT(effect_handler_context_t *context)
{
	msg("You fall into a pit!");
	if (player_of_has(player, OF_FEATHER)) {
		msg("You float gently to the bottom of the pit.");
	} else {
		take_hit(player, damroll(2, 6), "a trap");
	}
	wieldeds_notice_flag(player, OF_FEATHER);
	return TRUE;
}

bool effect_handler_TRAP_PIT_SPIKES(effect_handler_context_t *context)
{
	msg("You fall into a spiked pit!");

	if (player_of_has(player, OF_FEATHER)) {
		msg("You float gently to the floor of the pit.");
		msg("You carefully avoid touching the spikes.");
	} else {
		int dam = damroll(2, 6);

		/* Extra spike damage */
		if (one_in_(2)) {
			msg("You are impaled!");
			dam *= 2;
			(void)player_inc_timed(player, TMD_CUT, randint1(dam), TRUE, TRUE);
		}

		take_hit(player, dam, "a trap");
	}
	wieldeds_notice_flag(player, OF_FEATHER);
	return TRUE;
}

bool effect_handler_TRAP_PIT_POISON(effect_handler_context_t *context)
{
	msg("You fall into a spiked pit!");

	if (player_of_has(player, OF_FEATHER)) {
		msg("You float gently to the floor of the pit.");
		msg("You carefully avoid touching the spikes.");
	} else {
		int dam = damroll(2, 6);

		/* Extra spike damage */
		if (one_in_(2)) {
			msg("You are impaled on poisonous spikes!");
			(void)player_inc_timed(player, TMD_CUT, randint1(dam * 2), TRUE, TRUE);
			(void)player_inc_timed(player, TMD_POISONED, randint1(dam * 4), TRUE, TRUE);
		}

		take_hit(player, dam, "a trap");
	}
	wieldeds_notice_flag(player, OF_FEATHER);
	return TRUE;
}

bool effect_handler_TRAP_RUNE_SUMMON(effect_handler_context_t *context)
{
	int i;
	int num = 2 + randint1(3);

	msgt(MSG_SUM_MONSTER, "You are enveloped in a cloud of smoke!");

	/* Remove trap */
	sqinfo_off(cave->info[player->py][player->px], SQUARE_MARK);
	square_destroy_trap(cave, player->py, player->px);

	for (i = 0; i < num; i++)
		(void)summon_specific(player->py, player->px, player->depth, 0, 1);

	return TRUE;
}

bool effect_handler_TRAP_RUNE_TELEPORT(effect_handler_context_t *context)
{
	msg("You hit a teleport trap!");
	teleport_player(100);
	return TRUE;
}

bool effect_handler_TRAP_SPOT_FIRE(effect_handler_context_t *context)
{
	int dam = damroll(4, 6);
	msg("You are enveloped in flames!");
	dam = adjust_dam(GF_FIRE, dam, RANDOMISE, 0);
	if (dam) {
		take_hit(player, dam, "a fire trap");
		inven_damage(player, GF_FIRE, MIN(dam * 5, 300));
	}
	return TRUE;
}

bool effect_handler_TRAP_SPOT_ACID(effect_handler_context_t *context)
{
	int dam = damroll(4, 6);
	msg("You are splashed with acid!");
	dam = adjust_dam(GF_ACID, dam, RANDOMISE, 0);
	if (dam) {
		take_hit(player, dam, "an acid trap");
		inven_damage(player, GF_ACID, MIN(dam * 5, 300));
	}
	return TRUE;
}

bool effect_handler_TRAP_DART_SLOW(effect_handler_context_t *context)
{
	if (trap_check_hit(125)) {
		msg("A small dart hits you!");
		take_hit(player, damroll(1, 4), "a trap");
		(void)player_inc_timed(player, TMD_SLOW, randint0(20) + 20, TRUE, FALSE);
	} else {
		msg("A small dart barely misses you.");
	}
	return TRUE;
}

bool effect_handler_TRAP_DART_LOSE_STR(effect_handler_context_t *context)
{
	if (trap_check_hit(125)) {
		msg("A small dart hits you!");
		take_hit(player, damroll(1, 4), "a trap");
		(void)do_dec_stat(STAT_STR, FALSE);
	} else {
		msg("A small dart barely misses you.");
	}
	return TRUE;
}

bool effect_handler_TRAP_DART_LOSE_DEX(effect_handler_context_t *context)
{
	if (trap_check_hit(125)) {
		msg("A small dart hits you!");
		take_hit(player, damroll(1, 4), "a trap");
		(void)do_dec_stat(STAT_DEX, FALSE);
	} else {
		msg("A small dart barely misses you.");
	}
	return TRUE;
}

bool effect_handler_TRAP_DART_LOSE_CON(effect_handler_context_t *context)
{
	if (trap_check_hit(125)) {
		msg("A small dart hits you!");
		take_hit(player, damroll(1, 4), "a trap");
		(void)do_dec_stat(STAT_CON, FALSE);
	} else {
		msg("A small dart barely misses you.");
	}
	return TRUE;
}

bool effect_handler_TRAP_GAS_BLIND(effect_handler_context_t *context)
{
	msg("You are surrounded by a black gas!");
	(void)player_inc_timed(player, TMD_BLIND, randint0(50) + 25, TRUE, TRUE);
	return TRUE;
}

bool effect_handler_TRAP_GAS_CONFUSE(effect_handler_context_t *context)
{
	msg("You are surrounded by a gas of scintillating colors!");
	(void)player_inc_timed(player, TMD_CONFUSED, randint0(20) + 10, TRUE, TRUE);
	return TRUE;
}

bool effect_handler_TRAP_GAS_POISON(effect_handler_context_t *context)
{
	msg("You are surrounded by a pungent green gas!");
	(void)player_inc_timed(player, TMD_POISONED, randint0(20) + 10, TRUE, TRUE);
	return TRUE;
}

bool effect_handler_TRAP_GAS_SLEEP(effect_handler_context_t *context)
{
	msg("You are surrounded by a strange white mist!");
	(void)player_inc_timed(player, TMD_PARALYZED, randint0(10) + 5, TRUE, TRUE);
	return TRUE;
}


/**
 * Useful things about effects.
 */
static const struct effect_kind atomic_effects[] =
{
	{ AEF_ATOMIC_NONE, FALSE, NULL, NULL },
	#define F(x) effect_handler_##x
	#define EFFECT(x, a, d)    { AEF_##x, a, F(x), d },
	#include "list-atomic-effects.h"
	#undef EFFECT
	#undef F
	{ AEF_ATOMIC_MAX, FALSE, NULL, NULL }
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
		mem_free(e);
		e = e_next;
	}
}

bool effect_valid(struct effect *effect)
{
	return effect->index > AEF_ATOMIC_NONE && effect->index < AEF_ATOMIC_MAX;
}

bool effect_aim(struct effect *effect)
{
	struct effect *e = effect;

	if (!effect_valid(effect))
		return FALSE;

	while (e) {
		if (atomic_effects[e->index].aim) return TRUE;
		e = e->next;
	}

	return FALSE;
}

const char *effect_desc(struct effect *effect)
{
	if (!effect_valid(effect))
		return NULL;

	return atomic_effects[effect->index].desc;
}

atomic_effect_index atomic_effect_lookup(const char *name)
{
	static const char *effect_names[] = {
		#define EFFECT(x, a, d)	#x,
		#include "list-atomic-effects.h"
		#undef EFFECT
	};
	int i;

	for (i = 0; i < AEF_ATOMIC_MAX; i++) {
		const char *effect_name = effect_names[i];

		/* Test for equality */
		if (effect_name != NULL && streq(name, effect_name))
			return i;
	}

	return AEF_ATOMIC_MAX;
}

/*
 * Do an effect, given an object.
 * Boost is the extent to which skill surpasses difficulty, used as % boost. It
 * ranges from 0 to 138.
 */
bool atomic_effect_do(struct effect *effect, bool *ident, bool aware, int dir, int beam, int boost)
{
	bool handled = FALSE;
	effect_handler_f handler;
	random_value value;

	do {
		int random_choices, leftover = 0;

		if (!effect_valid(effect)) {
			msg("Bad effect passed to effect_do(). Please report this bug.");
			return FALSE;
		}

		if (effect->dice != NULL)
			random_choices = dice_roll(effect->dice, &value);

		/* Deal with special random effect */
		if (effect->index == AEF_ATOMIC_RANDOM) {
			int choice = randint0(random_choices);
			leftover = random_choices - choice;

			/* Skip to the chosen effect */
			effect = effect->next;
			while (choice--)
				effect = effect->next;
		}

		/* Handle the effect */
		handler = atomic_effects[effect->index].handler;
		if (handler != NULL) {
			effect_handler_context_t context = {
				effect->index,
				aware,
				dir,
				beam,
				boost,
				value,
				effect->params[0],
				effect->params[1],
				*ident,
			};

			handled = handler(&context);
			*ident = context.ident;
		}

		if (!handled) {
			msg("Effect not handled.");
			break;
		}

		/* Get the next effect, if there is one */
		if (leftover) 
			/* Skip the remaining non-chosen effects */
			while (leftover--)
				effect = effect->next;
		else
			effect = effect->next;
	} while (effect);

	return handled;
}
