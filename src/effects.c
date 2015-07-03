/**
 * \file effects.c
 * \brief Handler and auxiliary functions for every effect in the game
 *
 * Copyright (c) 2007 Andi Sidwell
 * Copyright (c) 2014 Ben Semmler, Nick McConnell
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
#include "game-input.h"
#include "generate.h"
#include "init.h"
#include "mon-desc.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-spell.h"
#include "mon-summon.h"
#include "mon-util.h"
#include "obj-chest.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-ignore.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-power.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-history.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"
#include "target.h"
#include "trap.h"


typedef struct effect_handler_context_s {
	const effect_index effect;
	const struct object *obj;
	const bool aware;
	const int dir;
	const int beam;
	const int boost;
	const random_value value;
	const int p1, p2, p3;
	bool ident;
} effect_handler_context_t;

typedef bool (*effect_handler_f)(effect_handler_context_t *);

/**
 * Structure for effects
 */
struct effect_kind {
	u16b index;          /* Effect index */
	bool aim;            /* Whether the effect requires aiming */
	const char *info;    /* Effect info (for spell tips) */
	effect_handler_f handler;    /* Function to perform the effect */
	const char *desc;    /* Effect description */
};


/**
 * Element info for player breaths
 */
static struct breath_info {
	const char *desc;    /* Element description */
	int msgt;            /* Element message type */
} elements[] = {
	#define ELEM(a, b, c, d, e, f, g, h, i, col) { c, i },
	#include "list-elements.h"
	#undef ELEM
};


/**
 * Array of stat adjectives
 */
static const char *desc_stat_pos[] =
{
	#define STAT(a, b, c, d, e, f, g, h) f,
	#include "list-stats.h"
	#undef STAT
};


/**
 * Array of stat opposite adjectives
 */
static const char *desc_stat_neg[] =
{
	#define STAT(a, b, c, d, e, f, g, h) g,
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
static bool project_aimed(int typ, int dir, int dam, int flg,
						  const struct object *obj)
{
	int py = player->py;
	int px = player->px;
	/* Player or monster? */
	int source = (cave->mon_current > 0) ? cave->mon_current : -1;

	int ty, tx;

	/* Pass through the target if needed */
	flg |= (PROJECT_THRU);

	/* Can hurt the player */
	if (source > 0)
		flg |= (PROJECT_PLAY);

	/* Use the adjacent grid in the given direction as target */
	ty = py + ddy[dir];
	tx = px + ddx[dir];

	/* Ask for a target if no direction given */
	if ((dir == 5) && target_okay() && source == -1)
		target_get(&tx, &ty);

	/* Aim at the target, do NOT explode */
	return (project(source, 0, ty, tx, dam, typ, flg, 0, 0, obj));
}

/**
 * Apply the project() function to grids the player is touching
 */
static bool project_touch(int dam, int typ, bool aware,
						  const struct object *obj)
{
	int py = player->py;
	int px = player->px;

	int flg = PROJECT_GRID | PROJECT_KILL | PROJECT_HIDE | PROJECT_ITEM | PROJECT_THRU;
	if (aware) flg |= PROJECT_AWARE;
	return (project(-1, 1, py, px, dam, typ, flg, 0, 0, obj));
}

/**
 * Dummy effect, to tell the effect code to pick one of the next 
 * context->value.base effects at random.
 */
bool effect_handler_RANDOM(effect_handler_context_t *context)
{
	return TRUE;
}

/**
 * Deal damage from the current monster to the player
 */
bool effect_handler_DAMAGE(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, FALSE);
	char ddesc[80];
	struct monster *mon;

	/* Get the monster */
	mon = cave_monster(cave, cave->mon_current);

	/* Get the "died from" name in case this attack kills @ */
	monster_desc(ddesc, sizeof(ddesc), mon, MDESC_DIED_FROM);

	/* Hit the player */
	take_hit(player, dam, ddesc);

	return TRUE;
}


/**
 * Heal the player by a given percentage of their wounds, or a minimum
 * amount, whichever is larger.
 *
 * context->value.base should be the minimum, and
 * context->value.m_bonus the percentage
 */
bool effect_handler_HEAL_HP(effect_handler_context_t *context)
{
	int num;

	/* Paranoia */
	if ((context->value.m_bonus <= 0) && (context->value.base <= 0))
		return (TRUE);

	/* Slight hack to ID !Life */
	if (context->value.base >= 5000) context->ident = TRUE;

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
 * Monster self-healing.
 */
bool effect_handler_MON_HEAL_HP(effect_handler_context_t *context)
{
	int midx = cave->mon_current;
	int amount = effect_calculate_value(context, FALSE);
	struct monster *mon = midx > 0 ? cave_monster(cave, midx) : NULL;
	char m_name[80], m_poss[80];
	bool seen;

	if (!mon) return TRUE;

	/* Get the monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), mon, MDESC_STANDARD);

	/* Get the monster possessive ("his"/"her"/"its") */
	monster_desc(m_poss, sizeof(m_poss), mon, MDESC_PRO_VIS | MDESC_POSS);

	seen = (!player->timed[TMD_BLIND] && mflag_has(mon->mflag, MFLAG_VISIBLE));

	/* Heal some */
	mon->hp += amount;

	/* Fully healed */
	if (mon->hp >= mon->maxhp) {
		mon->hp = mon->maxhp;

		if (seen)
			msg("%s looks REALLY healthy!", m_name);
		else
			msg("%s sounds REALLY healthy!", m_name);
	} else if (seen) { /* Partially healed */
		msg("%s looks healthier.", m_name);
	} else {
		msg("%s sounds healthier.", m_name);
	}

	/* Redraw (later) if needed */
	if (player->upkeep->health_who == mon)
		player->upkeep->redraw |= (PR_HEALTH);

	/* Cancel fear */
	if (mon->m_timed[MON_TMD_FEAR]) {
		mon_clear_timed(mon, MON_TMD_FEAR, MON_TMD_FLG_NOMESSAGE, FALSE);
		msg("%s recovers %s courage.", m_name, m_poss);
	}

	return TRUE;
}

/**
 * Feed the player.
 */
bool effect_handler_NOURISH(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, FALSE);
	player_set_food(player, player->food + amount);
	return TRUE;
}

bool effect_handler_CRUNCH(effect_handler_context_t *context)
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
bool effect_handler_CURE(effect_handler_context_t *context)
{
	int type = context->p1;
	if (player_clear_timed(player, type, TRUE))
		context->ident = TRUE;
	return TRUE;
}

/**
 * Set a (positive or negative) player status condition.
 */
bool effect_handler_TIMED_SET(effect_handler_context_t *context)
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
bool effect_handler_TIMED_INC(effect_handler_context_t *context)
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
 * Extend a (positive or negative) player status condition unresistably.
 * If context->p2 is set, increase by that amount if the status exists already
 */
bool effect_handler_TIMED_INC_NO_RES(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, FALSE);

	if (!player->timed[context->p1] || !context->p2)
		player_inc_timed(player, context->p1, amount, TRUE, FALSE);
	else
		player_inc_timed(player, context->p1, context->p2, TRUE, FALSE);
	context->ident = TRUE;
	return TRUE;

}

/**
 * Extend a (positive or negative) monster status condition.
 */
bool effect_handler_MON_TIMED_INC(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, FALSE);
	struct monster *mon  = cave->mon_current > 0 ?
		cave_monster(cave, cave->mon_current) : NULL;

	if (mon) {
		mon_inc_timed(mon, context->p1, amount, 0, FALSE);
		context->ident = TRUE;
	}
	return TRUE;

}

/**
 * Reduce a (positive or negative) player status condition.
 * If context->p2 is set, decrease by the current value / context->p2
 */
bool effect_handler_TIMED_DEC(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, FALSE);
	if (context->p2)
		amount = player->timed[context->p1] / context->p2;
	if (player_dec_timed(player, context->p1, amount, TRUE))
		context->ident = TRUE;
	return TRUE;

}

/**
 * Make the player, um, lose food.  Or gain it.
 */
bool effect_handler_SET_NOURISH(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, FALSE);
	if (player_set_food(player, amount))
		context->ident = TRUE;
	return TRUE;
}

bool effect_handler_CONFUSING(effect_handler_context_t *context)
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
bool effect_handler_RUNE(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;

	/* Always notice */
	context->ident = TRUE;

	/* See if the effect works */
	if (!square_canward(cave, py, px)) {
		msg("There is no clear floor on which to cast the spell.");
		return FALSE;
	}

	/* Create a glyph */
	square_add_ward(cave, py, px);

	/* Push objects off the grid */
	if (square_object(cave, py, px))
		push_object(py, px);

	return TRUE;
}

/**
 * Restore a stat.  The stat index is context->p1, message printed if
 * context->p2 is non-zero.
 */
bool effect_handler_RESTORE_STAT(effect_handler_context_t *context)
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
	update_stuff(player);

	/* Message */
	if (context->p2)
		msg("You feel less %s.", desc_stat_neg[stat]);

	/* Success */
	context->ident = TRUE;

	return (TRUE);
}

/**
 * Drain a stat temporarily.  The stat index is context->p1.
 */
bool effect_handler_DRAIN_STAT(effect_handler_context_t *context)
{
	int stat = context->p1;
	int flag = sustain_flag(stat);

	/* Bounds check */
	if (flag < 0) return FALSE;

	/* Sustain */
	if (player_of_has(player, flag)) {
		/* Notice effect */
		equip_notice_flag(player, flag);

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
		if (dam)
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
bool effect_handler_LOSE_RANDOM_STAT(effect_handler_context_t *context)
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
bool effect_handler_GAIN_STAT(effect_handler_context_t *context)
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
 * Restores any drained experience; message suppressed if context->p1 is set
 */
bool effect_handler_RESTORE_EXP(effect_handler_context_t *context)
{
	/* Restore experience */
	if (player->exp < player->max_exp) {
		/* Message */
		if (context->p1 == 0)
			msg("You feel your life energies returning.");
		player_exp_gain(player, player->max_exp - player->exp);

		/* Recalculate max. hitpoints */
		update_stuff(player);

		/* Did something */
		context->ident = TRUE;
	}

	return (TRUE);
}

/* Note the divisor of 2, a slight hack to simplify food description */
bool effect_handler_GAIN_EXP(effect_handler_context_t *context)
{
	int amount = effect_calculate_value(context, FALSE);
	if (player->exp < PY_MAX_EXP) {
		msg("You feel more experienced.");
		player_exp_gain(player, amount / 2);
		context->ident = TRUE;
	}
	return TRUE;
}

bool effect_handler_LOSE_EXP(effect_handler_context_t *context)
{
	if (!player_of_has(player, OF_HOLD_LIFE) && (player->exp > 0)) {
		msg("You feel your memories fade.");
		player_exp_lose(player, player->exp / 4, FALSE);
	}
	context->ident = TRUE;
	equip_notice_flag(player, OF_HOLD_LIFE);
	return TRUE;
}

/**
 * Drain mana from the player, healing the caster.
 */
bool effect_handler_DRAIN_MANA(effect_handler_context_t *context)
{
	int drain = effect_calculate_value(context, FALSE);
	char m_name[80];
	struct monster *mon = cave_monster(cave, cave->mon_current);

	if (!mon) return TRUE;

	/* Get the monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), mon, MDESC_STANDARD);

	if (!player->csp) {
		msg("The draining fails.");
		update_smart_learn(mon, player, 0, PF_NO_MANA, -1);
		return TRUE;
	}

	/* Full drain */
	if (drain >= player->csp) {
		drain = player->csp;
		player->csp = 0;
		player->csp_frac = 0;
	}
	/* Partial drain */
	else
		player->csp -= drain;

	/* Redraw mana */
	player->upkeep->redraw |= PR_MANA;

	/* Heal the monster */
	if (mon->hp < mon->maxhp) {
		mon->hp += (6 * drain);
		if (mon->hp > mon->maxhp)
			mon->hp = mon->maxhp;

		/* Redraw (later) if needed */
		if (player->upkeep->health_who == mon)
			player->upkeep->redraw |= (PR_HEALTH);

		/* Special message */
		if (mflag_has(mon->mflag, MFLAG_VISIBLE))
			msg("%s appears healthier.", m_name);
	}

	return TRUE;
}

bool effect_handler_RESTORE_MANA(effect_handler_context_t *context)
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

/*
 * Hack -- Removes curse from an object.
 */
static void uncurse_object(struct object *obj)
{
	bitflag f[OF_SIZE];

	create_mask(f, FALSE, OFT_CURSE, OFT_MAX);

	of_diff(obj->flags, f);
}


/*
 * Removes curses from items in inventory.
 *
 * \param heavy removes heavy curses if true
 *
 * \returns number of items uncursed
 */
static int remove_curse_aux(bool heavy)
{
	int i, cnt = 0;

	/* Attempt to uncurse items being worn */
	for (i = 0; i < player->body.count; i++) {
		struct object *obj = slot_object(player, i);

		if (!obj) continue;
		if (!cursed_p(obj->flags)) continue;

		/* Heavily cursed items need a special spell */
		if (of_has(obj->flags, OF_HEAVY_CURSE) && !heavy) continue;

		/* Perma-cursed items can never be removed */
		if (of_has(obj->flags, OF_PERMA_CURSE)) continue;

		/* Uncurse, and update things */
		uncurse_object(obj);

		player->upkeep->update |= (PU_BONUS);
		player->upkeep->redraw |= (PR_EQUIP);

		/* Count the uncursings */
		cnt++;
	}

	/* Return "something uncursed" */
	return (cnt);
}


/*
 * Remove most curses
 */
bool remove_curse(void)
{
	return (remove_curse_aux(FALSE));
}

/*
 * Remove all curses
 */
bool remove_all_curse(void)
{
	return (remove_curse_aux(TRUE));
}

/**
 * Will need revamping with curses - NRM
 */
bool effect_handler_REMOVE_CURSE(effect_handler_context_t *context)
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
bool effect_handler_REMOVE_ALL_CURSE(effect_handler_context_t *context)
{
	remove_all_curse();
	context->ident = TRUE;
	return TRUE;
}

/**
 * Set word of recall as appropriate
 */
bool effect_handler_RECALL(effect_handler_context_t *context)
{
	int target_depth;
	context->ident = TRUE;	

	/* No recall */
	if (OPT(birth_no_recall) && !player->total_winner) {
		msg("Nothing happens.");
		return TRUE;
	}

	/* No recall from quest levels with force_descend */
	if (OPT(birth_force_descend) && (is_quest(player->depth))) {
		msg("Nothing happens.");
		return TRUE;
	}

	/* Warn the player if they're descending to an unrecallable level */
	target_depth = dungeon_get_next_level(player->max_depth, 1);
	if (OPT(birth_force_descend) && !(player->depth) &&
			(is_quest(target_depth))) {
		if (!get_check("Are you sure you want to descend? ")) {
			return FALSE;
		}
	}

	/* Activate recall */
	if (!player->word_recall) {
		/* Reset recall depth */
		if ((player->depth > 0) && (player->depth != player->max_depth)) {
			/* ToDo: Add a new player field "recall_depth" */
			if (get_check("Reset recall depth? "))
				player->max_depth = player->depth;
		}

		player->word_recall = randint0(20) + 15;
		msg("The air about you becomes charged...");
	} else {
		/* Deactivate recall */
		if (!get_check("Word of Recall is already active.  Do you want to cancel it? "))
			return FALSE;

		player->word_recall = 0;
		msg("A tension leaves the air around you...");
	}

	/* Redraw status line */
	player->upkeep->redraw |= PR_STATUS;
	handle_stuff(player);

	return TRUE;
}

bool effect_handler_DEEP_DESCENT(effect_handler_context_t *context)
{
	int i, target_increment, target_depth = player->max_depth;

	/* Calculate target depth */
	target_increment = (4 / z_info->stair_skip) + 1;
	target_depth = dungeon_get_next_level(player->max_depth, target_increment);
	for (i = 5; i > 0; i--) {
		if (is_quest(target_depth)) break;
		if (target_depth >= z_info->max_depth - 1) break;

		target_depth++;
	}

	if (target_depth > player->depth) {
		msgt(MSG_TPLEVEL, "The air around you starts to swirl...");
		player->deep_descent = 3 + randint1(4);
		context->ident = TRUE;

		/* Redraw status line */
		player->upkeep->redraw |= PR_STATUS;
		handle_stuff(player);

		return TRUE;
	} else {
		msgt(MSG_TPLEVEL, "You sense a malevolent presence blocking passage to the levels below.");
		context->ident = TRUE;
		return TRUE;
	}
}

bool effect_handler_ALTER_REALITY(effect_handler_context_t *context)
{
	msg("The world changes!");
	dungeon_change_level(player->depth);

	return TRUE;
}

/**
 * Map an area around the player.  The height to map above and below the player
 * is context->value.dice, the width either side of the player
 * context->value.sides.
 *
 */
bool effect_handler_MAP_AREA(effect_handler_context_t *context)
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
			if (square_isno_map(cave, y, x)) continue;

			/* All non-walls are "checked" */
			if (!square_seemslikewall(cave, y, x)) {
				if (!square_in_bounds_fully(cave, y, x)) continue;

				/* Memorize normal features */
				if (!square_isfloor(cave, y, x)) {
					sqinfo_on(cave->squares[y][x].info, SQUARE_MARK);
					square_light_spot(cave, y, x);
				}

				/* Memorize known walls */
				for (i = 0; i < 8; i++) {
					int yy = y + ddy_ddd[i];
					int xx = x + ddx_ddd[i];

					/* Memorize walls (etc) */
					if (square_seemslikewall(cave, yy, xx)) {
						sqinfo_on(cave->squares[yy][xx].info, SQUARE_MARK);
						cave_k->squares[yy][xx].feat = cave->squares[yy][xx].feat;
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
bool effect_handler_DETECT_TRAPS(effect_handler_context_t *context)
{
	int x, y;
	int x1, x2, y1, y2;
	int y_dist = context->value.dice;
	int x_dist = context->value.sides;

	bool detect = FALSE;

	struct object *obj;

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
			if (square_isplayertrap(cave, y, x))
				/* Reveal trap */
				if (square_reveal_trap(cave, y, x, 100, FALSE))
					detect = TRUE;

			/* Scan all objects in the grid to look for traps on chests */
			for (obj = square_object(cave, y, x); obj; obj = obj->next) {
				/* Skip anything not a trapped chest */
				if (!is_trapped_chest(obj)) continue;

				/* Identify once */
				if (!object_is_known(obj)) {
					/* Know the trap */
					object_notice_everything(obj);

					/* Notice it */
					disturb(player, 0);

					/* We found something to detect */
					detect = TRUE;
				}
			}

			/* Mark as trap-detected */
			sqinfo_on(cave->squares[y][x].info, SQUARE_DTRAP);
		}
	}

	/* Rescan the map for the new dtrap edge */
	for (y = y1 - 1; y < y2 + 1; y++) {
		for (x = x1 - 1; x < x2 + 1; x++) {
			if (!square_in_bounds_fully(cave, y, x)) continue;

			/* See if this grid is on the edge */
			if (square_dtrap_edge(cave, y, x)) {
				sqinfo_on(cave->squares[y][x].info, SQUARE_DEDGE);
			} else {
				sqinfo_off(cave->squares[y][x].info, SQUARE_DEDGE);
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
bool effect_handler_DETECT_DOORS(effect_handler_context_t *context)
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
				sqinfo_on(cave->squares[y][x].info, SQUARE_MARK);
				cave_k->squares[y][x].feat = cave->squares[y][x].feat;
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
bool effect_handler_DETECT_STAIRS(effect_handler_context_t *context)
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
				sqinfo_on(cave->squares[y][x].info, SQUARE_MARK);
				cave_k->squares[y][x].feat = cave->squares[y][x].feat;
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
 * the player is context->value.dice, the width either side of the player
 * context->value.sides, and setting context->p1 to 1 suppresses messages.
 */
bool effect_handler_DETECT_GOLD(effect_handler_context_t *context)
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

			/* Magma/Quartz + Known Gold */
			if (square_hasgoldvein(cave, y, x)) {
				/* Hack -- Memorize */
				sqinfo_on(cave->squares[y][x].info, SQUARE_MARK);

				/* Redraw */
				square_light_spot(cave, y, x);

				/* Detect */
				gold_buried = TRUE;
				context->ident = TRUE;
			}
		}
	}

	/* Message unless we're silently detecting */
	if (context->p1 != 1) {
		if (gold_buried)
			msg("You sense the presence of buried treasure!");
		else if (context->aware)
			msg("You sense no buried treasure.");
	}

	return TRUE;
}

/**
 * Sense objects around the player.  The height to sense above and below the
 * player is context->value.dice, the width either side of the player
 * context->value.sides
 */
bool effect_handler_SENSE_OBJECTS(effect_handler_context_t *context)
{
	int x, y;
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

	/* Scan the area for objects */
	for (y = y1; y <= y2; y++) {
		for (x = x1; x <= x2; x++) {
			struct object *obj = square_object(cave, y, x);

			/* Skip empty grids */
			if (!obj) continue;

			/* Notice an object is detected */
			objects = TRUE;
			context->ident = TRUE;

			/* Mark the pile as aware */
			while (obj) {
				if (obj->marked == MARK_UNAWARE)
					obj->marked = MARK_AWARE;
				obj = obj->next;
			}

			/* Redraw */
			square_light_spot(cave, y, x);
		}
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
bool effect_handler_DETECT_OBJECTS(effect_handler_context_t *context)
{
	int x, y;
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

	/* Scan the area for objects */
	for (y = y1; y <= y2; y++) {
		for (x = x1; x <= x2; x++) {
			struct object *obj = square_object(cave, y, x);

			/* Skip empty grids */
			if (!obj) continue;

			/* Notice an object is detected */
			if (!ignore_item_ok(obj)) {
				objects = TRUE;
				context->ident = TRUE;
			}

			/* Markthe pile as seen */
			while (obj) {
				obj->marked = MARK_SEEN;
				obj = obj->next;
			}

			/* Redraw */
			square_light_spot(cave, y, x);
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
bool effect_handler_DETECT_VISIBLE_MONSTERS(effect_handler_context_t *context)
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
		struct monster *mon = cave_monster(cave, i);

		/* Skip dead monsters */
		if (!mon->race) continue;

		/* Location */
		y = mon->fy;
		x = mon->fx;

		/* Only detect nearby monsters */
		if (x < x1 || y < y1 || x > x2 || y > y2) continue;

		/* Detect all non-invisible, obvious monsters */
		if (!rf_has(mon->race->flags, RF_INVISIBLE) &&
			!mflag_has(mon->mflag, MFLAG_UNAWARE)) {
			/* Hack -- Detect the monster */
			mflag_on(mon->mflag, MFLAG_MARK);
			mflag_on(mon->mflag, MFLAG_SHOW);

			/* Update monster recall window */
			if (player->upkeep->monster_race == mon->race)
				/* Redraw stuff */
				player->upkeep->redraw |= (PR_MONSTER);

			/* Update the monster */
			update_mon(mon, cave, FALSE);

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
bool effect_handler_DETECT_INVISIBLE_MONSTERS(effect_handler_context_t *context)
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
		struct monster *mon = cave_monster(cave, i);
		struct monster_lore *lore;

		/* Skip dead monsters */
		if (!mon->race) continue;

		lore = get_lore(mon->race);

		/* Location */
		y = mon->fy;
		x = mon->fx;

		/* Only detect nearby monsters */
		if (x < x1 || y < y1 || x > x2 || y > y2) continue;

		/* Detect invisible monsters */
		if (rf_has(mon->race->flags, RF_INVISIBLE)) {
			/* Take note that they are invisible */
			rf_on(lore->flags, RF_INVISIBLE);

			/* Update monster recall window */
			if (player->upkeep->monster_race == mon->race)
				player->upkeep->redraw |= (PR_MONSTER);

			/* Detect the monster */
			mflag_on(mon->mflag, MFLAG_MARK);
			mflag_on(mon->mflag, MFLAG_SHOW);

			/* Update the monster */
			update_mon(mon, cave, FALSE);

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
bool effect_handler_DETECT_EVIL(effect_handler_context_t *context)
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
		struct monster *mon = cave_monster(cave, i);
		struct monster_lore *lore;

		/* Skip dead monsters */
		if (!mon->race) continue;

		lore = get_lore(mon->race);

		/* Location */
		y = mon->fy;
		x = mon->fx;

		/* Only detect nearby monsters */
		if (x < x1 || y < y1 || x > x2 || y > y2) continue;

		/* Detect evil monsters */
		if (rf_has(mon->race->flags, RF_EVIL)) {
			/* Take note that they are evil */
			rf_on(lore->flags, RF_EVIL);

			/* Update monster recall window */
			if (player->upkeep->monster_race == mon->race)
				player->upkeep->redraw |= (PR_MONSTER);

			/* Detect the monster */
			mflag_on(mon->mflag, MFLAG_MARK);
			mflag_on(mon->mflag, MFLAG_SHOW);

			/* Update the monster */
			update_mon(mon, cave, FALSE);

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
bool effect_handler_CREATE_STAIRS(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;

	context->ident = TRUE;

	/* Only allow stairs to be created on empty floor */
	if (!square_isfloor(cave, py, px)) {
		msg("There is no empty floor here.");
		return FALSE;
	}

	/* Push objects off the grid */
	if (square_object(cave, py, px))
		push_object(py, px);

	square_add_stairs(cave, py, px, player->depth);

	return TRUE;
}

/**
 * Apply disenchantment to the player's stuff.
 */
bool effect_handler_DISENCHANT(effect_handler_context_t *context)
{
	int i, count = 0;
	struct object *obj;
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
	obj = slot_object(player, i);

	/* No item, nothing happens */
	if (!obj) return TRUE;

	/* Nothing to disenchant */
	if ((obj->to_h <= 0) && (obj->to_d <= 0) && (obj->to_a <= 0))
		return TRUE;

	/* Describe the object */
	object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);

	/* Artifacts have a 60% chance to resist */
	if (obj->artifact && (randint0(100) < 60)) {
		/* Message */
		msg("Your %s (%c) resist%s disenchantment!", o_name, I2A(i),
			((obj->number != 1) ? "" : "s"));

		/* Notice */
		context->ident = TRUE;

		return TRUE;
	}

	/* Apply disenchantment, depending on which kind of equipment */
	if (slot_type_is(i, EQUIP_WEAPON) || slot_type_is(i, EQUIP_BOW)) {
		/* Disenchant to-hit */
		if (obj->to_h > 0) obj->to_h--;
		if ((obj->to_h > 5) && (randint0(100) < 20)) obj->to_h--;

		/* Disenchant to-dam */
		if (obj->to_d > 0) obj->to_d--;
		if ((obj->to_d > 5) && (randint0(100) < 20)) obj->to_d--;
	} else {
		/* Disenchant to-ac */
		if (obj->to_a > 0) obj->to_a--;
		if ((obj->to_a > 5) && (randint0(100) < 20)) obj->to_a--;
	}

	/* Message */
	msg("Your %s (%c) %s disenchanted!", o_name, I2A(i),
		((obj->number != 1) ? "were" : "was"));

	/* Recalculate bonuses */
	player->upkeep->update |= (PU_BONUS);

	/* Window stuff */
	player->upkeep->redraw |= (PR_EQUIP);

	/* Notice */
	context->ident = TRUE;

	return TRUE;
}

/**
 * Bit flags for the enchant() function
 */
#define ENCH_TOHIT   0x01
#define ENCH_TODAM   0x02
#define ENCH_TOBOTH  0x03
#define ENCH_TOAC	0x04

/**
 * Used by the enchant() function (chance of failure)
 */
static const int enchant_table[16] =
{
	0, 10,  20, 40, 80,
	160, 280, 400, 550, 700,
	800, 900, 950, 970, 990,
	1000
};

/**
 * Hook to specify "weapon"
 */
static bool item_tester_hook_weapon(const struct object *obj)
{
	return tval_is_weapon(obj);
}


/**
 * Hook to specify "armour"
 */
static bool item_tester_hook_armour(const struct object *obj)
{
	return tval_is_armor(obj);
}

/**
 * Tries to increase an items bonus score, if possible.
 *
 * \returns true if the bonus was increased
 */
static bool enchant_score(s16b *score, bool is_artifact)
{
	int chance;

	/* Artifacts resist enchantment half the time */
	if (is_artifact && randint0(100) < 50) return FALSE;

	/* Figure out the chance to enchant */
	if (*score < 0) chance = 0;
	else if (*score > 15) chance = 1000;
	else chance = enchant_table[*score];

	/* If we roll less-than-or-equal to chance, it fails */
	if (randint1(1000) <= chance) return FALSE;

	/* Increment the score */
	++*score;

	return TRUE;
}

/**
 * Tries to uncurse a cursed item, if possible
 *
 * \returns true if a curse was broken
 */
static bool enchant_curse(struct object *obj, bool is_artifact)
{
	/* If the item isn't cursed (or is perma-cursed) this doesn't work */
	if (!cursed_p(obj->flags) || of_has(obj->flags, OF_PERMA_CURSE)) 
		return FALSE;

	/* Artifacts resist enchanting curses away half the time */
	if (is_artifact && randint0(100) < 50) return FALSE;

	/* Normal items are uncursed 25% of the tiem */
	if (randint0(100) >= 25) return FALSE;

	/* Uncurse the item */
	msg("The curse is broken!");
	uncurse_object(obj);
	return TRUE;
}

/**
 * Helper function for enchant() which tries to do the two things that
 * enchanting an item does, namely increasing its bonuses and breaking curses
 *
 * \returns true if a bonus was increased or a curse was broken
 */
static bool enchant2(struct object *obj, s16b *score)
{
	bool result = FALSE;
	bool is_artifact = obj->artifact ? TRUE : FALSE;
	if (enchant_score(score, is_artifact)) result = TRUE;
	if (enchant_curse(obj, is_artifact)) result = TRUE;
	return result;
}

/**
 * Enchant an item
 *
 * Revamped!  Now takes item pointer, number of times to try enchanting, and a
 * flag of what to try enchanting.  Artifacts resist enchantment some of the
 * time. Also, any enchantment attempt (even unsuccessful) kicks off a parallel
 * attempt to uncurse a cursed item.
 *
 * Note that an item can technically be enchanted all the way to +15 if you
 * wait a very, very, long time.  Going from +9 to +10 only works about 5% of
 * the time, and from +10 to +11 only about 1% of the time.
 *
 * Note that this function can now be used on "piles" of items, and the larger
 * the pile, the lower the chance of success.
 *
 * \returns true if the item was changed in some way
 */
bool enchant(struct object *obj, int n, int eflag)
{
	int i, prob;
	bool res = FALSE;

	/* Large piles resist enchantment */
	prob = obj->number * 100;

	/* Missiles are easy to enchant */
	if (tval_is_ammo(obj)) prob = prob / 20;

	/* Try "n" times */
	for (i = 0; i < n; i++)
	{
		/* Roll for pile resistance */
		if (prob > 100 && randint0(prob) >= 100) continue;

		/* Try the three kinds of enchantment we can do */
		if ((eflag & ENCH_TOHIT) && enchant2(obj, &obj->to_h)) res = TRUE;
		if ((eflag & ENCH_TODAM) && enchant2(obj, &obj->to_d)) res = TRUE;
		if ((eflag & ENCH_TOAC)  && enchant2(obj, &obj->to_a)) res = TRUE;
	}

	/* Failure */
	if (!res) return (FALSE);

	/* Recalculate bonuses, gear */
	player->upkeep->update |= (PU_BONUS | PU_INVEN);

	/* Combine the pack (later) */
	player->upkeep->notice |= (PN_COMBINE);

	/* Redraw stuff */
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP );

	/* Success */
	return (TRUE);
}



/**
 * Enchant an item (in the inventory or on the floor)
 * Note that "num_ac" requires armour, else weapon
 * Returns TRUE if attempted, FALSE if cancelled
 *
 * Enchanting with the TOBOTH flag will try to enchant
 * both to_hit and to_dam with the same flag.  This
 * may not be the most desirable behavior (ACB).
 */
bool enchant_spell(int num_hit, int num_dam, int num_ac)
{
	bool okay = FALSE;

	struct object *obj;

	char o_name[80];

	const char *q, *s;

	/* Get an item */
	q = "Enchant which item? ";
	s = "You have nothing to enchant.";
	if (!get_item(&obj, q, s, 0, 
		num_ac ? item_tester_hook_armour : item_tester_hook_weapon,
		(USE_EQUIP | USE_INVEN | USE_QUIVER | USE_FLOOR)))
		return FALSE;

	/* Description */
	object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);

	/* Describe */
	msg("%s %s glow%s brightly!",
		(object_is_carried(player, obj) ? "Your" : "The"), o_name,
			   ((obj->number > 1) ? "" : "s"));

	/* Enchant */
	if (num_dam && enchant(obj, num_hit, ENCH_TOBOTH)) okay = TRUE;
	else if (enchant(obj, num_hit, ENCH_TOHIT)) okay = TRUE;
	else if (enchant(obj, num_dam, ENCH_TODAM)) okay = TRUE;
	if (enchant(obj, num_ac, ENCH_TOAC)) okay = TRUE;

	/* Failure */
	if (!okay) {
		event_signal(EVENT_INPUT_FLUSH);

		/* Message */
		msg("The enchantment failed.");
	}

	/* Something happened */
	return (TRUE);
}


/**
 * Brand weapons (or ammo)
 *
 * Turns the (non-magical) object into an ego-item of 'brand_type'.
 */
void brand_object(struct object *obj, const char *name)
{
	int i;
	struct ego_item *ego;
	bool ok = FALSE;

	/* you can never modify artifacts / ego-items */
	/* you can never modify cursed / worthless items */
	if (obj && !cursed_p(obj->flags) && obj->kind->cost &&
		!obj->artifact && !obj->ego) {
		char o_name[80];
		char brand[20];

		object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);
		strnfmt(brand, sizeof(brand), "of %s", name);

		/* Describe */
		msg("The %s %s surrounded with an aura of %s.", o_name,
			(obj->number > 1) ? "are" : "is", name);

		/* Get the right ego type for the object */
		for (i = 0; i < z_info->e_max; i++) {
			ego = &e_info[i];

			/* Match the name */
			if (!ego->name) continue;
			if (streq(ego->name, brand)) {
				struct ego_poss_item *poss;
				for (poss = ego->poss_items; poss; poss = poss->next)
					if (poss->kidx == obj->kind->kidx)
						ok = TRUE;
			}
			if (ok) break;
		}

		/* Make it an ego item */
		obj->ego = &e_info[i];
		ego_apply_magic(obj, 0);
		object_notice_ego(obj);

		/* Update the gear */
		player->upkeep->update |= (PU_INVEN);

		/* Combine the pack (later) */
		player->upkeep->notice |= (PN_COMBINE);

		/* Window stuff */
		player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);

		/* Enchant */
		enchant(obj, randint0(3) + 4, ENCH_TOHIT | ENCH_TODAM);
	} else {
		event_signal(EVENT_INPUT_FLUSH);
		msg("The branding failed.");
	}
}


/**
 * Enchant an item (in the inventory or on the floor)
 * Note that armour, to hit or to dam is controlled by context->p1
 *
 * Work on incorporating enchant_spell() has been postponed...NRM
 */
bool effect_handler_ENCHANT(effect_handler_context_t *context)
{
	int value = randcalc(context->value, player->depth, RANDOMISE);
	bool used = FALSE;
	context->ident = TRUE;

	if ((context->p1 & ENCH_TOBOTH) == ENCH_TOBOTH) {
		if (enchant_spell(value, value, 0))
			used = TRUE;
	}
	else if (context->p1 & ENCH_TOHIT) {
		if (enchant_spell(value, 0, 0))
			used = TRUE;
	}
	else if (context->p1 & ENCH_TODAM) {
		if (enchant_spell(0, value, 0))
			used = TRUE;
	}
	if (context->p1 & ENCH_TOAC) {
		if (enchant_spell(0, 0, value))
			used = TRUE;
	}

	return used;
}

/**
 * Hopefully this is OK now
 */
static bool item_tester_unknown(const struct object *obj)
{
	return object_is_known(obj) ? FALSE : TRUE;
}

/**
 * Identify an unknown item
 */
bool effect_handler_IDENTIFY(effect_handler_context_t *context)
{
	struct object *obj;
	const char *q, *s;
	bool used = FALSE;

	context->ident = TRUE;

	/* Get an item */
	q = "Identify which item? ";
	s = "You have nothing to identify.";
	if (!get_item(&obj, q, s, 0, item_tester_unknown,
				  (USE_EQUIP | USE_INVEN | USE_QUIVER | USE_FLOOR)))
		return used;

	/* Identify the object */
	do_ident_item(obj);

	return TRUE;
}

/**
 * Identify everything worn or carried by the player
 */
bool effect_handler_IDENTIFY_PACK(effect_handler_context_t *context)
{
	struct object *obj;

	context->ident = TRUE;

	/* Simply identify and know every item */
	for (obj = player->gear; obj; obj = obj->next) {
		/* Aware and Known */
		if (object_is_known(obj)) continue;

		/* Identify it */
		do_ident_item(obj);
	}

	return TRUE;
}

/*
 * Hook for "get_item()".  Determine if something is rechargable.
 */
static bool item_tester_hook_recharge(const struct object *obj)
{
	/* Recharge staves and wands */
	if (tval_can_have_charges(obj)) return TRUE;

	return FALSE;
}


/**
 * Recharge a wand or staff from the pack or on the floor.  Recharge strength
 * is context->value.base.
 *
 * It is harder to recharge high level, and highly charged wands.
 */
bool effect_handler_RECHARGE(effect_handler_context_t *context)
{
	int i, t, lev;
	int strength = context->value.base;
	struct object *obj;
	bool used = FALSE;
	const char *q, *s;

	/* Immediately obvious */
	context->ident = TRUE;

	/* Get an item */
	q = "Recharge which item? ";
	s = "You have nothing to recharge.";
	if (!get_item(&obj, q, s, 0, item_tester_hook_recharge,
				  (USE_INVEN | USE_FLOOR)))
		return (used);

	/* Extract the object "level" */
	lev = obj->kind->level;

	/* Chance of failure = 1 time in
	 * [Spell_strength + 100 - item_level - 10 * charge_per_item]/15 */
	i = (strength + 100 - lev - (10 * (obj->pval / obj->number))) / 15;

	/* Back-fire */
	if ((i <= 1) || one_in_(i)) {
		struct object *destroyed;
		bool none_left = FALSE;

		msg("The recharge backfires!");
		msg("There is a bright flash of light.");

		/* Reduce and describe inventory */
		if (object_is_carried(player, obj))
			destroyed = gear_object_for_use(obj, 1, TRUE, &none_left);
		else
			destroyed = floor_object_for_use(obj, 1, TRUE, &none_left);
		object_delete(&destroyed);
	} else {
		/* Extract a "power" */
		t = (strength / (lev + 2)) + 1;

		/* Recharge based on the power */
		if (t > 0) obj->pval += 2 + randint1(t);
	}

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
bool effect_handler_PROJECT_LOS(effect_handler_context_t *context)
{
	int i, x, y;
	int dam = effect_calculate_value(context, context->p2 ? TRUE : FALSE);
	int typ = context->p1;

	int flg = PROJECT_JUMP | PROJECT_KILL | PROJECT_HIDE;

	/* Affect all (nearby) monsters */
	for (i = 1; i < cave_monster_max(cave); i++) {
		struct monster *mon = cave_monster(cave, i);

		/* Paranoia -- Skip dead monsters */
		if (!mon->race) continue;

		/* Location */
		y = mon->fy;
		x = mon->fx;

		/* Require line of sight */
		if (!square_isview(cave, y, x)) continue;

		/* Jump directly to the target monster */
		if (project(-1, 0, y, x, dam, typ, flg, 0, 0, context->obj))
			context->ident = TRUE;
	}

	/* Result */
	return TRUE;
}

/**
 * Just like PROJECT_LOS except the player's awareness of an object using
 * this effect is relevant.
 *
 * Note that affected monsters are NOT auto-tracked by this usage.
 */
bool effect_handler_PROJECT_LOS_AWARE(effect_handler_context_t *context)
{
	int i, x, y;
	int dam = effect_calculate_value(context, context->p2 ? TRUE : FALSE);
	int typ = context->p1;

	int flg = PROJECT_JUMP | PROJECT_KILL | PROJECT_HIDE;

	if (context->aware) flg |= PROJECT_AWARE;

	/* Affect all (nearby) monsters */
	for (i = 1; i < cave_monster_max(cave); i++) {
		struct monster *mon = cave_monster(cave, i);

		/* Paranoia -- Skip dead monsters */
		if (!mon->race) continue;

		/* Location */
		y = mon->fy;
		x = mon->fx;

		/* Require line of sight */
		if (!square_isview(cave, y, x)) continue;

		/* Jump directly to the target monster */
		if (project(-1, 0, y, x, dam, typ, flg, 0, 0, context->obj))
			context->ident = TRUE;
	}

	/* Result */
	return TRUE;
}

bool effect_handler_ACQUIRE(effect_handler_context_t *context)
{
	int num = effect_calculate_value(context, FALSE);
	acquirement(player->py, player->px, player->depth, num, TRUE);
	context->ident = TRUE;
	return TRUE;
}

/**
 * Wake up all monsters, and speed up "los" monsters.
 *
 * Possibly the los test should be from the aggravating monster, rather than
 * automatically the player - NRM
 */
bool effect_handler_AGGRAVATE(effect_handler_context_t *context)
{
	int i;
	bool sleep = FALSE;
	int midx = cave->mon_current;
	struct monster *who = midx > 0 ? cave_monster(cave, midx) : NULL;

	/* Immediately obvious if the player did it */
	if (!who) {
		msg("There is a high pitched humming noise.");
		context->ident = TRUE;
	}

	/* Aggravate everyone nearby */
	for (i = 1; i < cave_monster_max(cave); i++) {
		struct monster *mon = cave_monster(cave, i);

		/* Paranoia -- Skip dead monsters */
		if (!mon->race) continue;

		/* Skip aggravating monster (or player) */
		if (mon == who) continue;

		/* Wake up nearby sleeping monsters */
		if ((mon->cdis < z_info->max_sight * 2) &&
			mon->m_timed[MON_TMD_SLEEP]) {
			mon_clear_timed(mon, MON_TMD_SLEEP, MON_TMD_FLG_NOMESSAGE, FALSE);
			sleep = TRUE;
			context->ident = TRUE;
		}

		/* Speed up monsters in line of sight */
		if (square_isview(cave, mon->fy, mon->fx)) {
			mon_inc_timed(mon, MON_TMD_FAST, 25, MON_TMD_FLG_NOTIFY, FALSE);
			if (is_mimicking(mon))
				become_aware(mon);
			context->ident = TRUE;
		}
	}

	/* Messages */
	if (sleep) msg("You hear a sudden stirring in the distance!");

	return TRUE;
}

/**
 * Summon context->value monsters of context->p1 type.
 */
bool effect_handler_SUMMON(effect_handler_context_t *context)
{
	int summon_max = effect_calculate_value(context, FALSE);
	int summon_type = context->p1 ? context->p1 : S_ANY;
	struct monster *mon = cave_monster(cave, cave->mon_current);
	int message_type = summon_message_type(summon_type);
	int count = 0, val = 0, attempts = 0;

	sound(message_type);

	/* Monster summon */
	if (mon) {
		int rlev = mon->race->level;

		/* Set the kin_base if necessary */
		if (summon_type == S_KIN)
			kin_base = mon->race->base;

		/* Continue summoning until we reach the current dungeon level */
		while ((val < player->depth * rlev) && (attempts < summon_max)) {
			int temp;

			/* Get a monster */
			temp = summon_specific(mon->fy, mon->fx, rlev, summon_type, FALSE,
								   FALSE);

			val += temp * temp;

			/* Increase the attempt in case no monsters were available. */
			attempts++;

			/* Increase count of summoned monsters */
			if (val > 0)
				count++;
		}

		/* In the special case that uniques or wraiths were summoned but all
		 * were dead S_HI_UNDEAD is used instead */
		if ((!count) &&
			((summon_type == S_WRAITH) || (summon_type == S_UNIQUE))) {
			attempts = 0;
			summon_type = S_HI_UNDEAD;
			while ((val < player->depth * rlev) && (attempts < summon_max)) {
				int temp;

				/* Get a monster */
				temp = summon_specific(mon->fy, mon->fx, rlev, summon_type,
									   FALSE, FALSE);

				val += temp * temp;

				/* Increase the attempt in case no monsters were available. */
				attempts++;

				/* Increase count of summoned monsters */
				if (val > 0)
					count++;
			}
		}
	} else {
		/* If not a monster summon, it's simple */
		while (summon_max) {
			count += summon_specific(player->py, player->px, player->depth,
									 summon_type, TRUE, FALSE);
			summon_max--;
		}
	}

	/* Identify if some monsters arrive */
	if (count)
		context->ident = TRUE;

	/* Message for the blind */
	if (count && player->timed[TMD_BLIND])
		msgt(message_type, "You hear %s appear nearby.",
			 (count > 1 ? "many things" : "something"));

	/* Summoner failed */
	if (mon && !count)
		msg("But nothing comes.");

	return TRUE;
}

/**
 * Delete all non-unique monsters of a given "type" from the level
 * -------
 * Warning - this function assumes that the entered monster symbol is an ASCII
 *		   character, which may not be true in the future - NRM
 * -------
 */
bool effect_handler_BANISH(effect_handler_context_t *context)
{
	int i;
	unsigned dam = 0;

	char typ;

	context->ident = TRUE;

	if (!get_com("Choose a monster race (by symbol) to banish: ", &typ))
		return FALSE;

	/* Delete the monsters of that "type" */
	for (i = 1; i < cave_monster_max(cave); i++) {
		struct monster *mon = cave_monster(cave, i);

		/* Paranoia -- Skip dead monsters */
		if (!mon->race) continue;

		/* Hack -- Skip Unique Monsters */
		if (rf_has(mon->race->flags, RF_UNIQUE)) continue;

		/* Skip "wrong" monsters (see warning above) */
		if ((char) mon->race->d_char != typ) continue;

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
bool effect_handler_MASS_BANISH(effect_handler_context_t *context)
{
	int i;
	int radius = context->p2 ? context->p2 : z_info->max_sight;
	unsigned dam = 0;

	context->ident = TRUE;

	/* Delete the (nearby) monsters */
	for (i = 1; i < cave_monster_max(cave); i++) {
		struct monster *mon = cave_monster(cave, i);

		/* Paranoia -- Skip dead monsters */
		if (!mon->race) continue;

		/* Hack -- Skip unique monsters */
		if (rf_has(mon->race->flags, RF_UNIQUE)) continue;

		/* Skip distant monsters */
		if (mon->cdis > radius) continue;

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

/**
 * Probe nearby monsters
 */
bool effect_handler_PROBE(effect_handler_context_t *context)
{
	int i;

	bool probe = FALSE;

	/* Probe all (nearby) monsters */
	for (i = 1; i < cave_monster_max(cave); i++) {
		struct monster *mon = cave_monster(cave, i);

		/* Paranoia -- Skip dead monsters */
		if (!mon->race) continue;

		/* Require line of sight */
		if (!square_isview(cave, mon->fy, mon->fx)) continue;

		/* Probe visible monsters */
		if (mflag_has(mon->mflag, MFLAG_VISIBLE)) {
			char m_name[80];

			/* Start the message */
			if (!probe) msg("Probing...");

			/* Get "the monster" or "something" */
			monster_desc(m_name, sizeof(m_name), mon,
					MDESC_IND_HID | MDESC_CAPITAL);

			/* Describe the monster */
			msg("%s has %d hit points.", m_name, mon->hp);

			/* Learn all of the non-spell, non-treasure flags */
			lore_do_probe(mon);

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
 * Thrust the player or a monster away from the source of a projection.   
 *
 * Monsters and players can be pushed past monsters or players weaker than 
 * they are.
 * If set, context->p1 and context->p2 act as y and x coordinates
 */
bool effect_handler_THRUST_AWAY(effect_handler_context_t *context)
{
	int y, x, yy, xx;
	int i, d, first_d;
	int angle;

	int c_y, c_x;

	int who = (cave->mon_current > 0) ? cave->mon_current : -1;
	int t_y = context->p1, t_x = context->p2;
	int grids_away = effect_calculate_value(context, FALSE);

	/*** Find a suitable endpoint for testing. ***/

	/* Get location of caster (assumes index of caster is not zero) */
	if (who > 0) {
		c_y = cave_monster(cave, who)->fy;
		c_x = cave_monster(cave, who)->fx;
	} else {
		c_y = player->py;
		c_x = player->px;
	}

	/* Ask for a target if none given */
	if (!(t_y && t_x))
		target_get(&t_x, &t_y);

	/* Determine where target is in relation to caster. */
	y = t_y - c_y + 20;
	x = t_x - c_x + 20;

	/* Find the angle (/2) of the line from caster to target. */
	angle = get_angle_to_grid[y][x];

	/* Start at the target grid. */
	y = t_y;
	x = t_x;

	/* Up to the number of grids requested, force the target away from the
	 * source of the projection, until it hits something it can't travel
	 * around. */
	for (i = 0; i < grids_away; i++) {
		/* Randomize initial direction. */
		first_d = randint0(8);

		/* Look around. */
		for (d = first_d; d < 8 + first_d; d++) {
			/* Reject angles more than 44 degrees from line. */
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
			yy = y + ddy_ddd[d % 8];
			xx = x + ddx_ddd[d % 8];

			/* Cannot switch places with stronger monsters. */
			if (cave->squares[yy][xx].mon != 0) {
				/* A monster is trying to pass. */
				if (cave->squares[y][x].mon > 0) {

					struct monster *mon = square_monster(cave, y, x);

					if (cave->squares[yy][xx].mon > 0) {
						struct monster *mon1 = square_monster(cave, yy, xx);

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
				if (cave->squares[y][x].mon < 0) {
					if (cave->squares[yy][xx].mon > 0) {
						struct monster *mon1 = square_monster(cave, yy, xx);

						/* Players cannot pass by stronger monsters. */
						if (mon1->race->level > player->lev * 2)
							continue;
					}
				}
			}

			/* Check for obstruction. */
			if (!square_isprojectable(cave, yy, xx)) {
				/* Some features allow entrance, but not exit. */
				if (square_ispassable(cave, yy, xx)) {
					/* Travel down the path. */
					monster_swap(y, x, yy, xx);

					/* Jump to new location. */
					y = yy;
					x = xx;

					/* We can't travel any more. */
					i = grids_away;

					/* Stop looking. */
					break;
				}

				/* If there are walls everywhere, stop here. */
				else if (d == (8 + first_d - 1)) {
					/* Message for player. */
					if (cave->squares[y][x].mon < 0)
						msg("You come to rest next to a wall.");
					i = grids_away;
				}
			} else {
				/* Travel down the path. */
				monster_swap(y, x, yy, xx);

				/* Jump to new location. */
				y = yy;
				x = xx;

				/* Stop looking at previous location. */
				break;
			}
		}
	}

	/* Clear the projection mark. */
	sqinfo_off(cave->squares[y][x].info, SQUARE_PROJECT);

	return TRUE;
}

/**
 * Teleport player or monster up to context->value.base grids away.
 *
 * If no spaces are readily available, the distance may increase.
 * Try very hard to move the player/monster at least a quarter that distance.
 * Setting context->p2 allows monsters to teleport the player away.
 * Setting context->p1 and context->p2 treats them as y and x coordinates
 * and teleports the monster from that grid.
 */
bool effect_handler_TELEPORT(effect_handler_context_t *context)
{
	int y_start = context->p1, x_start = context->p2;
	int dis = context->value.base;
	int d, i, min, y, x;
	int midx = cave->mon_current;
	struct monster *mon;

	bool look = TRUE;
	bool is_player = (midx < 0 || context->p2);

	context->ident = TRUE;

	/* Establish the coordinates to teleport from, if we don't know already */
	if (y_start && x_start) {
		/* We're good */
	} else if (is_player) {
		y_start = player->py;
		x_start = player->px;

		/* Check for a no teleport grid */
		if (square_isno_teleport(cave, y_start, x_start) && (dis > 10)) {
			msg("Teleportation forbidden!");
			return TRUE;
		}
	} else {
		mon = cave_monster(cave, midx);
		if (!mon->race) return TRUE;
		y_start = mon->fy;
		x_start = mon->fx;
	}

	/* Initialize */
	y = y_start;
	x = x_start;

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
				y = rand_spread(y_start, dis);
				x = rand_spread(x_start, dis);
				d = distance(y_start, x_start, y, x);
				if ((d >= min) && (d <= dis)) break;
			}

			/* Ignore illegal locations */
			if (!square_in_bounds_fully(cave, y, x)) continue;

			/* Require "naked" floor space */
			if (!square_isempty(cave, y, x)) continue;

			/* No teleporting into vaults and such */
			if (square_isvault(cave, y, x)) continue;

			/* No monster teleport onto glyph of warding */
			if (!is_player && square_iswarded(cave, y, x)) continue;

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
	sound(is_player ? MSG_TELEPORT : MSG_TPOTHER);

	/* Move player */
	monster_swap(y_start, x_start, y, x);

	/* Clear any projection marker to prevent double processing */
	sqinfo_off(cave->squares[y][x].info, SQUARE_PROJECT);

	/* Lots of updates after monster_swap */
	handle_stuff(player);

	return TRUE;
}

/**
 * Teleport player to a grid near the given location
 * Setting context->p1 and context->p2 treats them as y and x coordinates
 *
 * This function is slightly obsessive about correctness.
 * This function allows teleporting into vaults (!)
 */
bool effect_handler_TELEPORT_TO(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;

	int ny = py, nx = px;
	int y, x, dis = 0, ctr = 0;
	int midx = cave->mon_current;
	struct monster *mon;

	/* Initialize */
	y = py;
	x = px;

	context->ident = TRUE;

	/* Where are we going? */
	if (context->p1 && context->p2) {
		ny = context->p1;
		nx = context->p2;
	} else if (midx > 0) {
		mon = cave_monster(cave, midx);
		if (!mon) return TRUE;
		ny = mon->fy;
		nx = mon->fx;
	} else {
		if ((context->dir == 5) && target_okay())
			target_get(&nx, &ny);
	}

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

	/* Clear any projection marker to prevent double processing */
	sqinfo_off(cave->squares[y][x].info, SQUARE_PROJECT);

	/* Lots of updates after monster_swap */
	handle_stuff(player);

	return TRUE;
}

/**
 * Teleport the player one level up or down (random when legal)
 */
bool effect_handler_TELEPORT_LEVEL(effect_handler_context_t *context)
{
	bool up = TRUE, down = TRUE;
	int target_depth = dungeon_get_next_level(player->max_depth, 1);

	context->ident = TRUE;

	/* Resist hostile teleport */
	if ((cave->mon_current > 0) && player_resists(player, ELEM_NEXUS)) {
		msg("You resist the effect!");
		return TRUE;
	}

	/* No going up with force_descend or in the town */
	if (OPT(birth_force_descend) || !player->depth)
		up = FALSE;

	/* No forcing player down to quest levels if they can't leave */
	if (!up && is_quest(target_depth))
		down = FALSE;

	/* Can't leave quest levels or go down deeper than the dungeon */
	if (is_quest(player->depth) || (player->depth >= z_info->max_depth - 1))
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
		target_depth = dungeon_get_next_level(player->depth, -1);
		dungeon_change_level(target_depth);
	} else if (down) {
		msgt(MSG_TPLEVEL, "You sink through the floor.");

		if (OPT(birth_force_descend)) {
			target_depth = dungeon_get_next_level(player->max_depth, 1);
			dungeon_change_level(target_depth);
		} else {
			target_depth = dungeon_get_next_level(player->depth, 1);
			dungeon_change_level(target_depth);
		}
	} else {
		msg("Nothing happens.");
	}

	return TRUE;
}

/**
 * The destruction effect
 *
 * This effect "deletes" monsters (instead of killing them).
 *
 * This is always an effect centred on the player; it is similar to the
 * earthquake effect.
 */
bool effect_handler_DESTRUCTION(effect_handler_context_t *context)
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
			sqinfo_off(cave->squares[y][x].info, SQUARE_ROOM);
			sqinfo_off(cave->squares[y][x].info, SQUARE_VAULT);

			/* Lose light */
			sqinfo_off(cave->squares[y][x].info, SQUARE_GLOW);
			square_light_spot(cave, y, x);

			/* Deal with player later */
			if ((y == y1) && (x == x1)) continue;

			/* Delete the monster (if any) */
			delete_monster(y, x);

			/* Don't remove stairs */
			if (square_isstairs(cave, y, x)) continue;

			/* Lose knowledge (keeping knowledge of stairs) */
			sqinfo_off(cave->squares[y][x].info, SQUARE_MARK);

			/* Destroy any grid that isn't a permament wall */
			if (!square_isperm(cave, y, x)) {
				/* Deal with artifacts */
				struct object *obj = square_object(cave, y, x);
				while (obj) {
					if (obj->artifact) {
						if (!OPT(birth_no_preserve) && !object_was_sensed(obj))
							obj->artifact->created = FALSE;
						else
							history_lose_artifact(obj->artifact);
					}
					obj = obj->next;
				}

				/* Delete objects */
				square_excise_pile(cave, y, x);
				square_destroy(cave, y, x);
			}
		}
	}

	/* Message */
	msg("There is a searing blast of light!");

	/* Blind the player */
	equip_notice_element(player, ELEM_LIGHT);
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
 * Induce an earthquake of the radius context->p2 centred on the instigator.
 *
 * This will turn some walls into floors and some floors into walls.
 *
 * The player will take damage and jump into a safe grid if possible,
 * otherwise, he will tunnel through the rubble instantaneously.
 *
 * Monsters will take damage, and jump into a safe grid if possible,
 * otherwise they will be buried in the rubble, disappearing from
 * the level in the same way that they do when banished.
 *
 * Note that players and monsters (except eaters of walls and passers
 * through walls) will never occupy the same grid as a wall (or door).
 */
bool effect_handler_EARTHQUAKE(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;
	int r = context->p2;
	int i, y, x, yy, xx, dy, dx, cy, cx;
	int damage = 0;
	int safe_grids = 0, safe_y = 0, safe_x = 0;

	bool hurt = FALSE;
	bool map[32][32];

	context->ident = TRUE;

	/* Determine the epicentre */
	if (cave->mon_current > 0) {
		cy = cave_monster(cave, cave->mon_current)->fy;
		cx = cave_monster(cave, cave->mon_current)->fx;
	} else {
		cy = py;
		cx = px;
	}

	/* No effect in town */
	if (!player->depth) {
		msg("The ground shakes for a moment.");
		return TRUE;
	}

	/* Paranoia -- Enforce maximum range */
	if (r > 12) r = 12;

	/* Clear the "maximal blast" area */
	for (y = 0; y < 32; y++)
		for (x = 0; x < 32; x++)
			map[y][x] = FALSE;

	/* Check around the epicenter */
	for (dy = -r; dy <= r; dy++) {
		for (dx = -r; dx <= r; dx++) {
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip illegal grids */
			if (!square_in_bounds_fully(cave, yy, xx)) continue;

			/* Skip distant grids */
			if (distance(cy, cx, yy, xx) > r) continue;

			/* Lose room and vault */
			sqinfo_off(cave->squares[yy][xx].info, SQUARE_ROOM);
			sqinfo_off(cave->squares[yy][xx].info, SQUARE_VAULT);

			/* Lose light and knowledge */
			sqinfo_off(cave->squares[yy][xx].info, SQUARE_GLOW);
			sqinfo_off(cave->squares[yy][xx].info, SQUARE_MARK);

			/* Skip the epicenter */
			if (!dx && !dy) continue;

			/* Skip most grids */
			if (randint0(100) < 85) continue;

			/* Damage this grid */
			map[16 + yy - cy][16 + xx - cx] = TRUE;

			/* Hack -- Take note of player damage */
			if ((yy == py) && (xx == px)) hurt = TRUE;
		}
	}

	/* First, affect the player (if necessary) */
	if (hurt) {
		/* Check around the player */
		for (i = 0; i < 8; i++) {
			/* Get the location */
			y = py + ddy_ddd[i];
			x = px + ddx_ddd[i];

			/* Skip non-empty grids */
			if (!square_isempty(cave, y, x)) continue;

			/* Important -- Skip "quake" grids */
			if (map[16 + y - cy][16 + x - cx]) continue;

			/* Count "safe" grids, apply the randomizer */
			if ((++safe_grids > 1) && (randint0(safe_grids) != 0)) continue;

			/* Save the safe location */
			safe_y = y; safe_x = x;
		}

		/* Random message */
		switch (randint1(3))
		{
			case 1:
			{
				msg("The cave ceiling collapses!");
				break;
			}
			case 2:
			{
				msg("The cave floor twists in an unnatural way!");
				break;
			}
			default:
			{
				msg("The cave quakes!");
				msg("You are pummeled with debris!");
				break;
			}
		}

		/* Hurt the player a lot */
		if (!safe_grids) {
			/* Message and damage */
			msg("You are severely crushed!");
			damage = 300;
		} else {
			/* Destroy the grid, and push the player to (relative) safety */
			switch (randint1(3)) {
				case 1: {
					msg("You nimbly dodge the blast!");
					damage = 0;
					break;
				}
				case 2: {
					msg("You are bashed by rubble!");
					damage = damroll(10, 4);
					(void)player_inc_timed(player, TMD_STUN, randint1(50), TRUE, TRUE);
					break;
				}
				case 3: {
					msg("You are crushed between the floor and ceiling!");
					damage = damroll(10, 4);
					(void)player_inc_timed(player, TMD_STUN, randint1(50), TRUE, TRUE);
					break;
				}
			}

			/* Move player */
			monster_swap(py, px, safe_y, safe_x);
		}

		/* Take some damage */
		if (damage) take_hit(player, damage, "an earthquake");
	}


	/* Examine the quaked region */
	for (dy = -r; dy <= r; dy++) {
		for (dx = -r; dx <= r; dx++) {
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip unaffected grids */
			if (!map[16 + yy - cy][16 + xx - cx]) continue;

			/* Process monsters */
			if (cave->squares[yy][xx].mon > 0) {
				struct monster *mon = square_monster(cave, yy, xx);

				/* Most monsters cannot co-exist with rock */
				if (!flags_test(mon->race->flags, RF_SIZE, RF_KILL_WALL,
								RF_PASS_WALL, FLAG_END)) {
					char m_name[80];

					/* Assume not safe */
					safe_grids = 0;

					/* Monster can move to escape the wall */
					if (!rf_has(mon->race->flags, RF_NEVER_MOVE)) {
						/* Look for safety */
						for (i = 0; i < 8; i++) {
							/* Get the grid */
							y = yy + ddy_ddd[i];
							x = xx + ddx_ddd[i];

							/* Skip non-empty grids */
							if (!square_isempty(cave, y, x)) continue;

							/* Hack -- no safety on glyph of warding */
							if (square_iswarded(cave, y, x))
								continue;

							/* Important -- Skip quake grids */
							if (map[16 + y - cy][16 + x - cx]) continue;

							/* Count safe grids, apply the randomizer */
							if ((++safe_grids > 1) &&
								(randint0(safe_grids) != 0))
								continue;

							/* Save the safe grid */
							safe_y = y;
							safe_x = x;
						}
					}

					/* Describe the monster */
					monster_desc(m_name, sizeof(m_name), mon, MDESC_STANDARD);

					/* Scream in pain */
					msg("%s wails out in pain!", m_name);

					/* Take damage from the quake */
					damage = (safe_grids ? damroll(4, 8) : (mon->hp + 1));

					/* Monster is certainly awake */
					mon_clear_timed(mon, MON_TMD_SLEEP,
							MON_TMD_FLG_NOMESSAGE, FALSE);

					/* If the quake finished the monster off, show message */
					if (mon->hp < damage && mon->hp >= 0)
						msg("%s is embedded in the rock!", m_name);

					/* Apply damage directly */
					mon->hp -= damage;

					/* Delete (not kill) "dead" monsters */
					if (mon->hp < 0) {
						/* Delete the monster */
						delete_monster(yy, xx);

						/* No longer safe */
						safe_grids = 0;
					}

					/* Escape from the rock */
					if (safe_grids)
						/* Move the monster */
						monster_swap(yy, xx, safe_y, safe_x);
				}
			}
		}
	}

	/* Player may have moved */
	py = player->py;
	px = player->px;

	/* Important -- no wall on player */
	map[16 + py - cy][16 + px - cx] = FALSE;


	/* Examine the quaked region */
	for (dy = -r; dy <= r; dy++) {
		for (dx = -r; dx <= r; dx++) {
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Ignore invalid grids */
			if (!square_in_bounds_fully(cave, yy, xx)) continue;

			/* Note unaffected grids for light changes, etc. */
			if (!map[16 + yy - cy][16 + xx - cx])
				square_light_spot(cave, yy, xx);

			/* Destroy location and all objects (if valid) */
			else if (square_changeable(cave, yy, xx)) {
				square_excise_pile(cave, yy, xx);
				square_earthquake(cave, yy, xx);
			}
		}
	}

	/* Fully update the visuals */
	player->upkeep->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS);

	/* Fully update the flow */
	player->upkeep->update |= (PU_FORGET_FLOW | PU_UPDATE_FLOW);

	/* Update the health bar */
	player->upkeep->redraw |= (PR_HEALTH);

	/* Window stuff */
	player->upkeep->redraw |= (PR_MONLIST | PR_ITEMLIST);

	return TRUE;
}

bool effect_handler_ENLIGHTENMENT(effect_handler_context_t *context)
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
bool effect_handler_LIGHT_AREA(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;
	int dam = effect_calculate_value(context, FALSE);
	int rad = context->p2 + (context->p3 ? player->lev / context->p3 : 0);

	int flg = PROJECT_GRID | PROJECT_KILL;

	/* Message */
	if (!player->timed[TMD_BLIND])
		msg("You are surrounded by a white light.");

	/* Hook into the "project()" function */
	(void)project(-1, rad, py, px, dam, GF_LIGHT_WEAK, flg, 0, 0, context->obj);

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
bool effect_handler_DARKEN_AREA(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;
	int dam = effect_calculate_value(context, FALSE);
	int rad = context->p2;
	int source = (cave->mon_current > 0) ? cave->mon_current : -1;

	int flg = PROJECT_GRID | PROJECT_KILL | PROJECT_PLAY;

	/* Message */
	if (!player->timed[TMD_BLIND])
		msg("Darkness surrounds you.");

	/* Hook into the "project()" function */
	(void)project(source, rad, py, px, dam, GF_DARK_WEAK, flg, 0, 0,
				  context->obj);

	/* Darken the room */
	light_room(py, px, FALSE);

	/* Hack - blind the player directly if player-cast */
	if ((source == -1) && !player_resists(player, ELEM_DARK))
		(void)player_inc_timed(player, TMD_BLIND, 3 + randint1(5), TRUE, TRUE);

	/* Assume seen */
	context->ident = TRUE;
	return (TRUE);
}

/**
 * Cast a ball spell
 * Stop if we hit a monster or the player, act as a ball
 * Allow target mode to pass over monsters
 * Affect grids, objects, and monsters
 */
bool effect_handler_BALL(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;
	int dam = effect_calculate_value(context, TRUE);
	int rad = context->p2 ? context->p2 : 2;
	int source;

	int ty = py + ddy[context->dir];
	int tx = px + ddx[context->dir];

	int flg = PROJECT_THRU | PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	/* Player or monster? */
	if (cave->mon_current > 0) {
		struct monster *mon = cave_monster(cave, cave->mon_current);
		source = cave->mon_current;
		if (rf_has(mon->race->flags, RF_POWERFUL)) rad++;
		flg |= PROJECT_PLAY;
		flg &= ~(PROJECT_STOP | PROJECT_THRU);
	} else {
		if (context->p3) rad += player->lev / context->p3;
		source = -1;
	}

	/* Ask for a target if no direction given */
	if ((context->dir == 5) && target_okay() && source == -1) {
		flg &= ~(PROJECT_STOP | PROJECT_THRU);

		target_get(&tx, &ty);
	}

	/* Aim at the target, explode */
	if (project(source, rad, ty, tx, dam, context->p1, flg, 0, 0, context->obj))
		context->ident = TRUE;

	return TRUE;
}


/**
 * Breathe an element, in a cone from the breather
 * Affect grids, objects, and monsters
 * context->p1 is element, context->p2 degrees of arc, context->p3 radius
 */
bool effect_handler_BREATH(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;
	int dam = effect_calculate_value(context, TRUE);
	int type = context->p1;
	int rad = context->p3;
	int source;

	int ty = py + ddy[context->dir];
	int tx = px + ddx[context->dir];

	/* Diameter of source starts at 20, so full strength only adjacent to
	 * the breather. */
	int diameter_of_source = 20;
	int degrees_of_arc = context->p2;

	int flg = PROJECT_ARC | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	/* Radius of zero means no fixed limit. */
	if (rad == 0)
		rad = z_info->max_range;

	/* Player or monster? */
	if (cave->mon_current > 0) {
		struct monster *mon = cave_monster(cave, cave->mon_current);
		source = cave->mon_current;
		flg |= PROJECT_PLAY;

		dam = breath_dam(type, mon->hp);

		/* Powerful monsters breathe wider arcs */
		if (rf_has(mon->race->flags, RF_POWERFUL)) {
			diameter_of_source *= 2;
			degrees_of_arc *= 2;
		}
	} else {
		msgt(elements[type].msgt, "You breathe %s.", elements[type].desc);
		source = -1;
	}

	/* Ask for a target if no direction given */
	if ((context->dir == 5) && target_okay() && source == -1)
		target_get(&tx, &ty);

	/* Diameter of the energy source. */
	if (degrees_of_arc < 60) {
		if (degrees_of_arc == 0)
			/* This handles finite length beams */
			diameter_of_source = rad * 10;
		else
			/* Narrower cone means energy drops off less quickly. 30 degree
			 * breaths are still full strength 3 grids from the breather,
			 * and 20 degree breaths are still full strength at 5 grids. */
			diameter_of_source = diameter_of_source * 60 / degrees_of_arc;
	}

	/* Max */
	if (diameter_of_source > 250)
		diameter_of_source = 250;

	/* Breathe at the target */
	if (project(source, rad, ty, tx, dam, type, flg, degrees_of_arc,
				diameter_of_source, context->obj))
		context->ident = TRUE;

	return TRUE;
}


/**
 * Cast multiple non-jumping ball spells at the same target.
 *
 * Targets absolute coordinates instead of a specific monster, so that
 * the death of the monster doesn't change the target's location.
 */
bool effect_handler_SWARM(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;
	int dam = effect_calculate_value(context, TRUE);
	int num = context->value.m_bonus;

	int ty = py + ddy[context->dir];
	int tx = px + ddx[context->dir];

	int flg = PROJECT_THRU | PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	/* Ask for a target if no direction given (early detonation) */
	if ((context->dir == 5) && target_okay()) {
		flg &= ~(PROJECT_STOP | PROJECT_THRU);

		target_get(&tx, &ty);
	}

	while (num--) {
		/* Aim at the target.  Hurt items on floor. */
		if (project(-1, context->p2, ty, tx, dam, context->p1, flg, 0, 0,
					context->obj))
			context->ident = TRUE;
	}

	return TRUE;
}

/**
 * Cast a line spell in every direction
 * Stop if we hit a monster, act as a ball
 * Affect grids, objects, and monsters
 */
bool effect_handler_STAR(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;
	int dam = effect_calculate_value(context, TRUE);
	int i;

	s16b ty, tx;

	int flg = PROJECT_THRU | PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL;

	/* Describe */
	if (!player->timed[TMD_BLIND])
		msg("Light shoots in all directions!");

	for (i = 0; i < 8; i++) {
		/* Use the current direction */
		ty = py + ddy_ddd[i];
		tx = px + ddx_ddd[i];

		/* Aim at the target */
		if (project(-1, 0, ty, tx, dam, context->p1, flg, 0, 0, context->obj))
			context->ident = TRUE;
	}
	return TRUE;
}


/**
 * Cast a ball spell in every direction
 * Stop if we hit a monster, act as a ball
 * Affect grids, objects, and monsters
 */
bool effect_handler_STAR_BALL(effect_handler_context_t *context)
{
	int py = player->py;
	int px = player->px;
	int dam = effect_calculate_value(context, TRUE);
	int i;

	s16b ty, tx;

	int flg = PROJECT_STOP | PROJECT_THRU | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	for (i = 0; i < 8; i++) {
		/* Use the current direction */
		ty = py + ddy_ddd[i];
		tx = px + ddx_ddd[i];

		/* Aim at the target, explode */
		if (project(-1, context->p2, ty, tx, dam, context->p1, flg, 0, 0,
					context->obj))
			context->ident = TRUE;
	}
	return TRUE;
}

/**
 * Cast a bolt spell
 * Stop if we hit a monster, as a bolt
 * Affect monsters (not grids or objects)
 */
bool effect_handler_BOLT(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, TRUE);
	int flg = PROJECT_STOP | PROJECT_KILL;
	(void) project_aimed(context->p1, context->dir, dam, flg, context->obj);
	if (!player->timed[TMD_BLIND])
		context->ident = TRUE;
	return TRUE;
}

/**
 * Cast a beam spell
 * Pass through monsters, as a beam
 * Affect monsters (not grids or objects)
 */
bool effect_handler_BEAM(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, TRUE);
	int flg = PROJECT_BEAM | PROJECT_KILL;
	(void) project_aimed(context->p1, context->dir, dam, flg, context->obj);
	if (!player->timed[TMD_BLIND])
		context->ident = TRUE;
	return TRUE;
}

/**
 * Cast a bolt spell, or rarely, a beam spell
 * context->p2 is any adjustment to the regular beam chance
 * context->p3 being set means to divide by the adjustment instead of adding
 */
bool effect_handler_BOLT_OR_BEAM(effect_handler_context_t *context)
{
	int beam = context->beam;

	if (context->p3)
		beam /= context->p2;
	else
		beam += context->p2;

	if (randint0(100) < beam)
		return effect_handler_BEAM(context);
	else
		return effect_handler_BOLT(context);
}

/**
 * Cast a line spell
 * Pass through monsters, as a beam
 * Affect monsters and grids (not objects)
 */
bool effect_handler_LINE(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, TRUE);
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL;
	if (project_aimed(context->p1, context->dir, dam, flg, context->obj))
		context->ident = TRUE;
	return TRUE;
}

/**
 * Cast an alter spell
 * Affect objects and grids (not monsters)
 */
bool effect_handler_ALTER(effect_handler_context_t *context)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM;
	if (project_aimed(context->p1, context->dir, 0, flg, context->obj))
		context->ident = TRUE;
	return TRUE;
}

/**
 * Cast a bolt spell
 * Stop if we hit a monster, as a bolt
 * Affect monsters (not grids or objects)
 * Like BOLT, but only identifies on noticing an effect
 */
bool effect_handler_BOLT_STATUS(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, TRUE);
	int flg = PROJECT_STOP | PROJECT_KILL;
	if (project_aimed(context->p1, context->dir, dam, flg, context->obj))
		context->ident = TRUE;
	return TRUE;
}

/**
 * Cast a bolt spell
 * Stop if we hit a monster, as a bolt
 * Affect monsters (not grids or objects)
 * The same as BOLT_STATUS, but done as a separate function to aid descriptions
 */
bool effect_handler_BOLT_STATUS_DAM(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, TRUE);
	int flg = PROJECT_STOP | PROJECT_KILL;
	if (project_aimed(context->p1, context->dir, dam, flg, context->obj))
		context->ident = TRUE;
	return TRUE;
}

/**
 * Cast a bolt spell
 * Stop if we hit a monster, as a bolt
 * Affect monsters (not grids or objects)
 * Notice stuff based on awareness of the effect
 */
bool effect_handler_BOLT_AWARE(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, TRUE);
	int flg = PROJECT_STOP | PROJECT_KILL;
	if (context->aware) flg |= PROJECT_AWARE;
	if (project_aimed(context->p1, context->dir, dam, flg, context->obj))
		context->ident = TRUE;
	return TRUE;
}

/**
 * Affect adjacent grids (radius 1 ball attack)
 */
bool effect_handler_TOUCH(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, TRUE);
	if (project_touch(dam, context->p1, FALSE, context->obj))
		context->ident = TRUE;
	return TRUE;
}

/**
 * Affect adjacent grids (radius 1 ball attack)
 * Notice stuff based on awareness of the effect
 */
bool effect_handler_TOUCH_AWARE(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, TRUE);
	if (project_touch(dam, context->p1, context->aware, context->obj))
		context->ident = TRUE;
	return TRUE;
}

/**
 * Curse the player's armor
 */
bool effect_handler_CURSE_ARMOR(effect_handler_context_t *context)
{
	struct object *obj;

	char o_name[80];

	/* Curse the body armor */
	obj = equipped_item_by_slot_name(player, "body");

	/* Nothing to curse */
	if (!obj) return (TRUE);

	/* Describe */
	object_desc(o_name, sizeof(o_name), obj, ODESC_FULL);

	/* Attempt a saving throw for artifacts */
	if (obj->artifact && (randint0(100) < 50))
		/* Cool */
		msg("A %s tries to %s, but your %s resists the effects!",
				   "terrible black aura", "surround your armor", o_name);

	/* not artifact or failed save... */
	else {
		/* Oops */
		msg("A terrible black aura blasts your %s!", o_name);

		/* Take down bonus a wee bit */
		obj->to_a -= randint1(3);

		/* Curse it */
		flags_set(obj->flags, OF_SIZE, OF_LIGHT_CURSE, OF_HEAVY_CURSE, FLAG_END);

		/* Recalculate bonuses */
		player->upkeep->update |= (PU_BONUS);

		/* Recalculate mana */
		player->upkeep->update |= (PU_MANA);

		/* Window stuff */
		player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);
	}

	context->ident = TRUE;

	return (TRUE);
}


/**
 * Curse the player's weapon
 */
bool effect_handler_CURSE_WEAPON(effect_handler_context_t *context)
{
	struct object *obj;

	char o_name[80];

	/* Curse the weapon */
	obj = equipped_item_by_slot_name(player, "weapon");

	/* Nothing to curse */
	if (!obj) return (TRUE);

	/* Describe */
	object_desc(o_name, sizeof(o_name), obj, ODESC_FULL);

	/* Attempt a saving throw */
	if (obj->artifact && (randint0(100) < 50))
		/* Cool */
		msg("A %s tries to %s, but your %s resists the effects!",
				   "terrible black aura", "surround your weapon", o_name);

	/* not artifact or failed save... */
	else {
		/* Oops */
		msg("A terrible black aura blasts your %s!", o_name);

		/* Hurt it a bit */
		obj->to_h = 0 - randint1(3);
		obj->to_d = 0 - randint1(3);

		/* Curse it */
		flags_set(obj->flags, OF_SIZE, OF_LIGHT_CURSE, OF_HEAVY_CURSE, FLAG_END);

		/* Recalculate bonuses */
		player->upkeep->update |= (PU_BONUS);

		/* Recalculate mana */
		player->upkeep->update |= (PU_MANA);

		/* Window stuff */
		player->upkeep->redraw |= (PR_INVEN | PR_EQUIP);
	}

	context->ident = TRUE;

	/* Notice */
	return (TRUE);
}


/**
 * Brand the current weapon
 */
bool effect_handler_BRAND_WEAPON(effect_handler_context_t *context)
{
	struct object *obj = equipped_item_by_slot_name(player, "weapon");

	/* Select the brand */
	const char *brand = one_in_(2) ? "Flame" : "Frost";

	/* Brand the weapon */
	brand_object(obj, brand);

	context->ident = TRUE;
	return TRUE;
}


/*
 * Hook to specify "ammo"
 */
static bool item_tester_hook_ammo(const struct object *obj)
{
	return tval_is_ammo(obj);
}


/**
 * Brand some (non-magical) ammo
 */
bool effect_handler_BRAND_AMMO(effect_handler_context_t *context)
{
	struct object *obj;
	const char *q, *s;
	bool used = FALSE;

	/* Select the brand */
	const char *brand = one_in_(3) ? "Flame" : (one_in_(2) ? "Frost" : "Venom");

	context->ident = TRUE;

	/* Get an item */
	q = "Brand which kind of ammunition? ";
	s = "You have nothing to brand.";
	if (!get_item(&obj, q, s, 0, item_tester_hook_ammo, (USE_INVEN | USE_QUIVER | USE_FLOOR)))
		return used;

	/* Brand the ammo */
	brand_object(obj, brand);

	/* Done */
	return (TRUE);
}

static bool item_tester_hook_bolt(const struct object *obj)
{
	return obj->tval == TV_BOLT;
}

/**
 * Enchant some (non-magical) bolts
 */
bool effect_handler_BRAND_BOLTS(effect_handler_context_t *context)
{
	struct object *obj;
	const char *q, *s;
	bool used = FALSE;

	context->ident = TRUE;

	/* Get an item */
	q = "Brand which bolts? ";
	s = "You have no bolts to brand.";
	if (!get_item(&obj, q, s, 0, item_tester_hook_bolt, (USE_INVEN | USE_QUIVER | USE_FLOOR)))
		return used;

	/* Brand the bolts */
	brand_object(obj, "Flame");

	/* Done */
	return (TRUE);
}


/**
 * One Ring activation
 */
bool effect_handler_BIZARRE(effect_handler_context_t *context)
{
	context->ident = TRUE;
	/* Pick a random effect */
	switch (randint1(10))
	{
		case 1:
		case 2:
		{
			/* Message */
			msg("You are surrounded by a malignant aura.");

			/* Decrease all stats (permanently) */
			player_stat_dec(player, STAT_STR, TRUE);
			player_stat_dec(player, STAT_INT, TRUE);
			player_stat_dec(player, STAT_WIS, TRUE);
			player_stat_dec(player, STAT_DEX, TRUE);
			player_stat_dec(player, STAT_CON, TRUE);

			/* Lose some experience (permanently) */
			player_exp_lose(player, player->exp / 4, TRUE);

			return TRUE;
		}

		case 3:
		{
			struct effect *effect = mem_zalloc(sizeof(*effect));

			/* Message */
			msg("You are surrounded by a powerful aura.");

			/* Dispel monsters */
			effect_simple(EF_PROJECT_LOS, "1000", GF_DISP_ALL, 0, 0, NULL);

			return TRUE;
		}

		case 4:
		case 5:
		case 6:
		{
			/* Mana Ball */
			int flg = PROJECT_THRU | PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
			int ty = player->py + ddy[context->dir];
			int tx = player->px + ddx[context->dir];

			/* Ask for a target if no direction given */
			if ((context->dir == 5) && target_okay()) {
				flg &= ~(PROJECT_STOP | PROJECT_THRU);

				target_get(&tx, &ty);
			}

			/* Aim at the target, explode */
			if (project(-1, 3, ty, tx, 300, GF_MANA, flg, 0, 0, context->obj))

			return TRUE;
		}

		case 7:
		case 8:
		case 9:
		case 10:
		{
			/* Mana Bolt */
			int flg = PROJECT_STOP | PROJECT_KILL | PROJECT_THRU;
			int ty = player->py + ddy[context->dir];
			int tx = player->px + ddx[context->dir];

			/* Use an actual target */
			if ((context->dir == 5) && target_okay())
				target_get(&tx, &ty);

			/* Aim at the target, do NOT explode */
			return (project(-1, 0, ty, tx, 250, GF_MANA, flg, 0, 0,
							context->obj));

			return TRUE;
		}
	}
	return FALSE;
}

/**
 * The "wonder" effect.
 *
 * This spell should become more useful (more
 * controlled) as the player gains experience levels.
 * Thus, add 1/5 of the player's level to the die roll.
 * This eliminates the worst effects later on, while
 * keeping the results quite random.  It also allows
 * some potent effects only at high level
 */
bool effect_handler_WONDER(effect_handler_context_t *context)
{
	int plev = player->lev;
	int die = effect_calculate_value(context, FALSE);
	int p1 = 0, p2 = 0, p3 = 0;
	int beam = context->beam;
	effect_handler_f handler = NULL;
	random_value value = { 0, 0, 0, 0 };
	bool *ident = mem_zalloc(sizeof(*ident));

	context->ident = TRUE;

	if (die > 100)
		msg("You feel a surge of power!");

	if (die < 8) {
		p1 = GF_OLD_CLONE;
		handler = effect_handler_BOLT;
	} else if (die < 14) {
		p1 = GF_OLD_SPEED;
		value.base = 100;
		handler = effect_handler_BOLT;
	} else if (die < 26) {
		p1 = GF_OLD_HEAL;
		value.dice = 4;
		value.sides = 6;
		handler = effect_handler_BOLT;
	} else if (die < 31) {
		p1 = GF_OLD_POLY;
		value.base = plev;
		handler = effect_handler_BOLT;
	} else if (die < 36) {
		beam -= 10;
		p1 = GF_MISSILE;
		value.dice = 3 + ((plev - 1) / 5);
		value.sides = 4;
		handler = effect_handler_BOLT_OR_BEAM;
	} else if (die < 41) {
		p1 = GF_OLD_CONF;
		value.base = plev;
		handler = effect_handler_BOLT;
	} else if (die < 46) {
		p1 = GF_POIS;
		value.base = 20 + plev / 2;
		p2 = 3;
		handler = effect_handler_BALL;
	} else if (die < 51) {
		p1 = GF_LIGHT_WEAK;
		value.dice = 6;
		value.sides = 8;
		handler = effect_handler_LINE;
	} else if (die < 56) {
		p1 = GF_ELEC;
		value.dice = 3 + ((plev - 5) / 6);
		value.sides = 6;
		handler = effect_handler_BEAM;
	} else if (die < 61) {
		beam -= 10;
		p1 = GF_COLD;
		value.dice = 5 + ((plev - 5) / 4);
		value.sides = 8;
		handler = effect_handler_BOLT_OR_BEAM;
	} else if (die < 66) {
		p1 = GF_ACID;
		value.dice = 6 + ((plev - 5) / 4);
		value.sides = 8;
		handler = effect_handler_BOLT_OR_BEAM;
	} else if (die < 71) {
		p1 = GF_FIRE;
		value.dice = 8 + ((plev - 5) / 4);
		value.sides = 8;
		handler = effect_handler_BOLT_OR_BEAM;
	} else if (die < 76) {
		p1 = GF_OLD_DRAIN;
		value.base = 75;
		handler = effect_handler_BOLT;
	} else if (die < 81) {
		p1 = GF_ELEC;
		value.base = 30 + plev / 2;
		p2 = 2;
		handler = effect_handler_BALL;
	} else if (die < 86) {
		p1 = GF_ACID;
		value.base = 40 + plev;
		p2 = 2;
		handler = effect_handler_BALL;
	} else if (die < 91) {
		p1 = GF_ICE;
		value.base = 70 + plev;
		p2 = 3;
		handler = effect_handler_BALL;
	} else if (die < 96) {
		p1 = GF_FIRE;
		value.base = 80 + plev;
		p2 = 3;
		handler = effect_handler_BALL;
	} else if (die < 101) {
		p1 = GF_OLD_DRAIN;
		value.base = 100 + plev;
		handler = effect_handler_BOLT;
	} else if (die < 104) {
		p2 = 12;
		handler = effect_handler_EARTHQUAKE;
	} else if (die < 106) {
		p2 = 15;
		handler = effect_handler_DESTRUCTION;
	} else if (die < 108) {
		handler = effect_handler_BANISH;
	} else if (die < 110) {
		p1 = GF_DISP_ALL;
		value.base = 120;
		handler = effect_handler_PROJECT_LOS;
	}

	if (handler != NULL) {
		effect_handler_context_t new_context = {
			context->effect,
			context->obj,
			context->aware,
			context->dir,
			beam,
			context->boost,
			value,
			p1, p2, p3,
			ident
		};

		mem_free(ident);
		return (handler(&new_context));
	}

	/* RARE */
	effect_simple(EF_PROJECT_LOS, "150", GF_DISP_ALL, 0, 0, ident);
	effect_simple(EF_PROJECT_LOS, "20", GF_OLD_SLOW, 0, 0, ident);
	effect_simple(EF_PROJECT_LOS, "40", GF_OLD_SLEEP, 0, 0, ident);
	effect_simple(EF_HEAL_HP, "300", 0, 0, 0, ident);
	mem_free(ident);

	return TRUE;
}

bool effect_handler_TRAP_DOOR(effect_handler_context_t *context)
{
	int target_depth = dungeon_get_next_level(player->depth, 1);

	if (target_depth == player->depth) {
		msg("You feel quite certain something really awful just happened...");
		return TRUE;
	}

	msg("You fall through a trap door!");
	if (player_of_has(player, OF_FEATHER)) {
		msg("You float gently down to the next level.");
	} else {
		int dam = effect_calculate_value(context, FALSE);
		take_hit(player, dam, "a trap");
	}
	equip_notice_flag(player, OF_FEATHER);

	dungeon_change_level(target_depth);
	return TRUE;
}

bool effect_handler_TRAP_PIT(effect_handler_context_t *context)
{
	msg("You fall into a pit!");
	if (player_of_has(player, OF_FEATHER)) {
		msg("You float gently to the bottom of the pit.");
	} else {
		int dam = effect_calculate_value(context, FALSE);
		take_hit(player, dam, "a trap");
	}
	equip_notice_flag(player, OF_FEATHER);
	return TRUE;
}

bool effect_handler_TRAP_PIT_SPIKES(effect_handler_context_t *context)
{
	msg("You fall into a spiked pit!");

	if (player_of_has(player, OF_FEATHER)) {
		msg("You float gently to the floor of the pit.");
		msg("You carefully avoid touching the spikes.");
	} else {
		int dam = effect_calculate_value(context, FALSE);

		/* Extra spike damage */
		if (one_in_(2)) {
			msg("You are impaled!");
			dam *= 2;
			(void)player_inc_timed(player, TMD_CUT, randint1(dam), TRUE, TRUE);
		}

		take_hit(player, dam, "a trap");
	}
	equip_notice_flag(player, OF_FEATHER);
	return TRUE;
}

bool effect_handler_TRAP_PIT_POISON(effect_handler_context_t *context)
{
	msg("You fall into a spiked pit!");

	if (player_of_has(player, OF_FEATHER)) {
		msg("You float gently to the floor of the pit.");
		msg("You carefully avoid touching the spikes.");
	} else {
		int dam = effect_calculate_value(context, FALSE);

		/* Extra spike damage */
		if (one_in_(2)) {
			msg("You are impaled on poisonous spikes!");
			(void)player_inc_timed(player, TMD_CUT, randint1(dam * 2),
								   TRUE, TRUE);
			(void)player_inc_timed(player, TMD_POISONED, randint1(dam * 4),
								   TRUE, TRUE);
		}

		take_hit(player, dam, "a trap");
	}
	equip_notice_flag(player, OF_FEATHER);
	return TRUE;
}

bool effect_handler_TRAP_RUNE_SUMMON(effect_handler_context_t *context)
{
	int i;
	int num = effect_calculate_value(context, FALSE);

	msgt(MSG_SUM_MONSTER, "You are enveloped in a cloud of smoke!");

	/* Remove trap */
	sqinfo_off(cave->squares[player->py][player->px].info, SQUARE_MARK);
	square_destroy_trap(cave, player->py, player->px);

	for (i = 0; i < num; i++)
		(void)summon_specific(player->py, player->px, player->depth, 0, TRUE,
							  FALSE);

	return TRUE;
}

bool effect_handler_TRAP_RUNE_TELEPORT(effect_handler_context_t *context)
{
	int radius = effect_calculate_value(context, FALSE);
	char dist[5];
	strnfmt(dist, sizeof(dist), "%d", radius);
	msg("You hit a teleport trap!");
	effect_simple(EF_TELEPORT, dist, 0, 1, 0, NULL);
	return TRUE;
}

bool effect_handler_TRAP_SPOT_FIRE(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, FALSE);
	msg("You are enveloped in flames!");
	dam = adjust_dam(player, GF_FIRE, dam, RANDOMISE, 0);
	if (dam) {
		take_hit(player, dam, "a fire trap");
		inven_damage(player, GF_FIRE, MIN(dam * 5, 300));
	}
	return TRUE;
}

bool effect_handler_TRAP_SPOT_ACID(effect_handler_context_t *context)
{
	int dam = effect_calculate_value(context, FALSE);
	msg("You are splashed with acid!");
	dam = adjust_dam(player, GF_ACID, dam, RANDOMISE, 0);
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
		effect_simple(EF_DRAIN_STAT, "0", STAT_STR, 0, 0, NULL);
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
		effect_simple(EF_DRAIN_STAT, "0", STAT_DEX, 0, 0, NULL);
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
		effect_simple(EF_DRAIN_STAT, "0", STAT_CON, 0, 0, NULL);
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
static const struct effect_kind effects[] =
{
	{ EF_NONE, FALSE, NULL, NULL, NULL },
	#define F(x) effect_handler_##x
	#define EFFECT(x, a, b, c, d, e)	{ EF_##x, a, b, F(x), e },
	#include "list-effects.h"
	#undef EFFECT
	#undef F
	{ EF_MAX, FALSE, NULL, NULL, NULL }
};


static const char *effect_names[] = {
	NULL,
	#define EFFECT(x, a, b, c, d, e)	#x,
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
		mem_free(e);
		e = e_next;
	}
}

bool effect_valid(struct effect *effect)
{
	if (!effect) return FALSE;
	return effect->index > EF_NONE && effect->index < EF_MAX;
}

bool effect_aim(struct effect *effect)
{
	struct effect *e = effect;

	if (!effect_valid(effect))
		return FALSE;

	while (e) {
		if (effects[e->index].aim) return TRUE;
		e = e->next;
	}

	return FALSE;
}

const char *effect_info(struct effect *effect)
{
	if (!effect_valid(effect))
		return NULL;

	return effects[effect->index].info;
}

const char *effect_desc(struct effect *effect)
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
 * Translate a string to an effect parameter index
 */
int effect_param(int index, const char *type)
{
	int val = -1;

	/* If not a numerical value, assign according to effect index */
	if (sscanf(type, "%d", &val) != 1) {
		switch (index) {
				/* Projection name */
			case EF_PROJECT_LOS:
			case EF_PROJECT_LOS_AWARE:
			case EF_BALL:
			case EF_BREATH:
			case EF_SWARM:
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
			case EF_TOUCH:
			case EF_TOUCH_AWARE: {
				val = gf_name_to_idx(type);
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

				/* Anything else shoulcn't be calling this */
			default:
				;
		}
	}

	return val;
}

/**
 * Do an effect, given an object.
 * Boost is the extent to which skill surpasses difficulty, used as % boost. It
 * ranges from 0 to 138.
 *
 * Note that no effect ever sets `*ident` to FALSE
 */
bool effect_do(struct effect *effect, struct object *obj, bool *ident,
			   bool aware, int dir, int beam, int boost)
{
	bool completed = FALSE;
	effect_handler_f handler;
	random_value value = { 0, 0, 0, 0 };

	do {
		int random_choices = 0, leftover = 0;

		if (!effect_valid(effect)) {
			msg("Bad effect passed to effect_do(). Please report this bug.");
			return FALSE;
		}

		if (effect->dice != NULL)
			random_choices = dice_roll(effect->dice, &value);

		/* Deal with special random effect */
		if (effect->index == EF_RANDOM) {
			int choice = randint0(random_choices);
			leftover = random_choices - choice;

			/* Skip to the chosen effect */
			effect = effect->next;
			while (choice--)
				effect = effect->next;

			/* Roll the damage, if needed */
			if (effect->dice != NULL)
				(void) dice_roll(effect->dice, &value);
		}

		/* Handle the effect */
		handler = effects[effect->index].handler;
		if (handler != NULL) {
			effect_handler_context_t context = {
				effect->index,
				obj,
				aware,
				dir,
				beam,
				boost,
				value,
				effect->params[0],
				effect->params[1],
				effect->params[2],
				*ident,
			};

			completed = handler(&context) || completed;
			*ident = context.ident;
		}

		/* Get the next effect, if there is one */
		if (leftover) 
			/* Skip the remaining non-chosen effects */
			while (leftover--)
				effect = effect->next;
		else
			effect = effect->next;
	} while (effect);

	return completed;
}

/**
 * Perform a single effect with a simple dice string and parameters
 * Calling with ident a valid pointer will (depending on effect) give success
 * information; ident = NULL will ignore this 
 */
void effect_simple(int index, const char* dice_string, int p1, int p2, int p3, bool *ident)
{
	struct effect *effect = mem_zalloc(sizeof(*effect));
	int dir = DIR_TARGET;
	bool dummy_ident;

	/* Set all the values */
	effect->index = index;
	effect->dice = dice_new();
	dice_parse_string(effect->dice, dice_string);
	effect->params[0] = p1;
	effect->params[1] = p2;
	effect->params[2] = p3;

	/* Direction if needed */
	if (effect_aim(effect))
		get_aim_dir(&dir);

	/* Do the effect */
	if (ident)
		effect_do(effect, NULL, ident, TRUE, dir, 0, 0);
	else
		effect_do(effect, NULL, &dummy_ident, TRUE, dir, 0, 0);

	free_effect(effect);
}
