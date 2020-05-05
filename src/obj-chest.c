/**
 * \file obj-chest.c
 * \brief Encapsulation of chest-related functions
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2012 Peter Denison
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
#include "mon-lore.h"
#include "obj-chest.h"
#include "obj-ignore.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-spell.h"
#include "player-timed.h"
#include "player-util.h"

/**
 * Chest traps are specified in the file chest_trap.txt.
 *
 * Chests are described by their 16-bit pval as follows:
 * - pval of 0 is an empty chest
 * - pval of 1 is a locked chest with no traps
 * - pval > 1  is a trapped chest, with each bit of the pval aside from the
 *             lowest and highest (potentially) representing a different trap
 * - pval < 1  is a disarmed/unlocked chest; the disarming process is simply
 *             to negate the pval
 *
 * The chest pval also determines the difficulty of disarming the chest.
 * Currently the maximum difficulty is 60 (32 + 16 + 8 + 4); if more traps are
 * added to chest_trap.txt, the disarming calculation will need adjusting.
 */

struct chest_trap *chest_traps;
struct chest *chests;

/**
 * ------------------------------------------------------------------------
 * Parsing functions for chest_trap.txt and chest.txt
 * ------------------------------------------------------------------------ */
static enum parser_error parse_chest_trap_name(struct parser *p)
{
    const char *name = parser_getstr(p, "name");
    struct chest_trap *h = parser_priv(p);
    struct chest_trap *t = mem_zalloc(sizeof *t);

	/* Order the traps correctly and set the pval */
	if (h) {
		h->next = t;
		t->pval = h->pval * 2;
	} else {
		chest_traps = t;
		t->pval = 1;
	}
    t->name = string_make(name);
    parser_setpriv(p, t);
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_chest_trap_code(struct parser *p)
{
    const char *code = parser_getstr(p, "code");
    struct chest_trap *t = parser_priv(p);

    if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
    t->code = string_make(code);
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_chest_trap_level(struct parser *p)
{
    struct chest_trap *t = parser_priv(p);

    if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
    t->level = parser_getint(p, "level");
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_chest_trap_effect(struct parser *p) {
    struct chest_trap *t = parser_priv(p);
	struct effect *effect;
	struct effect *new_effect = mem_zalloc(sizeof(*new_effect));

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* Go to the next vacant effect and set it to the new one  */
	if (t->effect) {
		effect = t->effect;
		while (effect->next)
			effect = effect->next;
		effect->next = new_effect;
	} else
		t->effect = new_effect;

	/* Fill in the detail */
	return grab_effect_data(p, new_effect);
}

static enum parser_error parse_chest_trap_dice(struct parser *p) {
	struct chest_trap *t = parser_priv(p);
	dice_t *dice = NULL;
	struct effect *effect = t->effect;
	const char *string = NULL;

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;

	dice = dice_new();

	if (dice == NULL)
		return PARSE_ERROR_INVALID_DICE;

	string = parser_getstr(p, "dice");

	if (dice_parse_string(dice, string)) {
		effect->dice = dice;
	}
	else {
		dice_free(dice);
		return PARSE_ERROR_INVALID_DICE;
	}

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_chest_trap_expr(struct parser *p) {
	struct chest_trap *t = parser_priv(p);
	struct effect *effect = t->effect;
	expression_t *expression = NULL;
	expression_base_value_f function = NULL;
	const char *name;
	const char *base;
	const char *expr;

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;

	/* If there is no effect, assume that this is human and not parser error. */
	if (effect == NULL)
		return PARSE_ERROR_NONE;

	while (effect->next) effect = effect->next;

	/* If there are no dice, assume that this is human and not parser error. */
	if (effect->dice == NULL)
		return PARSE_ERROR_NONE;

	name = parser_getsym(p, "name");
	base = parser_getsym(p, "base");
	expr = parser_getstr(p, "expr");
	expression = expression_new();

	if (expression == NULL)
		return PARSE_ERROR_INVALID_EXPRESSION;

	function = spell_value_base_by_name(base);
	expression_set_base_value(expression, function);

	if (expression_add_operations_string(expression, expr) < 0)
		return PARSE_ERROR_BAD_EXPRESSION_STRING;

	if (dice_bind_expression(effect->dice, name, expression) < 0)
		return PARSE_ERROR_UNBOUND_EXPRESSION;

	/* The dice object makes a deep copy of the expression, so we can free it */
	expression_free(expression);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_chest_trap_destroy(struct parser *p) {
    struct chest_trap *t = parser_priv(p);
	int val = 0;

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
    val = parser_getint(p, "val");
	if (val) {
		t->destroy = true;
	}
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_chest_trap_magic(struct parser *p) {
    struct chest_trap *t = parser_priv(p);
	int val = 0;

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
    val = parser_getint(p, "val");
	if (val) {
		t->magic = true;
	}
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_chest_trap_msg(struct parser *p) {
    struct chest_trap *t = parser_priv(p);

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
    t->msg = string_append(t->msg, parser_getstr(p, "text"));
    return PARSE_ERROR_NONE;
}

static enum parser_error parse_chest_trap_msg_death(struct parser *p) {
    struct chest_trap *t = parser_priv(p);

	if (!t)
		return PARSE_ERROR_MISSING_RECORD_HEADER;
    t->msg_death = string_append(t->msg_death, parser_getstr(p, "text"));
    return PARSE_ERROR_NONE;
}

struct parser *init_parse_chest_trap(void) {
    struct parser *p = parser_new();
    parser_setpriv(p, NULL);
    parser_reg(p, "name str name", parse_chest_trap_name);
    parser_reg(p, "code str code", parse_chest_trap_code);
    parser_reg(p, "level int level", parse_chest_trap_level);
	parser_reg(p, "effect sym eff ?sym type ?int radius ?int other", parse_chest_trap_effect);
	parser_reg(p, "dice str dice", parse_chest_trap_dice);
	parser_reg(p, "expr sym name sym base str expr", parse_chest_trap_expr);
	parser_reg(p, "destroy int val", parse_chest_trap_destroy);
	parser_reg(p, "magic int val", parse_chest_trap_magic);
	parser_reg(p, "msg str text", parse_chest_trap_msg);
	parser_reg(p, "msg-death str text", parse_chest_trap_msg_death);
    return p;
}

static errr run_parse_chest_trap(struct parser *p) {
    return parse_file_quit_not_found(p, "chest_trap");
}

static errr finish_parse_chest_trap(struct parser *p) {
	parser_destroy(p);
	return 0;
}

static void cleanup_chest_trap(void)
{
	struct chest_trap *trap = chest_traps;
	while (trap) {
		struct chest_trap *old = trap;
		string_free(trap->name);
		string_free(trap->code);
		string_free(trap->msg);
		string_free(trap->msg_death);
		free_effect(trap->effect);
		trap = trap->next;
		mem_free(old);
	}
}

struct file_parser chest_trap_parser = {
    "chest_trap",
    init_parse_chest_trap,
    run_parse_chest_trap,
    finish_parse_chest_trap,
    cleanup_chest_trap
};

/**
 * ------------------------------------------------------------------------
 * Chest trap information
 * ------------------------------------------------------------------------ */
/**
 * The name of a chest trap
 */
char *chest_trap_name(const struct object *obj)
{
	s16b trap_value = obj->pval;

	/* Non-zero value means there either were or are still traps */
	if (trap_value < 0) {
		return (trap_value == -1) ? "unlocked" : "disarmed";
	} else if (trap_value > 0) {
		struct chest_trap *trap = chest_traps, *found = NULL;
		while (trap) {
			if (trap_value & trap->pval) {
				if (found) {
					return "multiple traps";
				}
				found = trap;
			}
			trap = trap->next;
		}
		if (found) {
			return found->name;
		}
	}

	return "empty";
}

/**
 * Determine if a chest is trapped
 */
bool is_trapped_chest(const struct object *obj)
{
	if (!tval_is_chest(obj))
		return false;

	/* Ignore if requested */
	if (ignore_item_ok(obj))
		return false;

	/* Disarmed or opened chests are not trapped */
	if (obj->pval <= 0)
		return false;

	/* Some chests simply don't have traps */
	return (obj->pval == 1) ? false : true;
}


/**
 * Determine if a chest is locked or trapped
 */
bool is_locked_chest(const struct object *obj)
{
	if (!tval_is_chest(obj))
		return false;

	/* Ignore if requested */
	if (ignore_item_ok(obj))
		return false;

	/* Disarmed or opened chests are not locked */
	return (obj->pval > 0);
}

/**
 * ------------------------------------------------------------------------
 * Chest trap actions
 * ------------------------------------------------------------------------ */
/**
 * Pick a single chest trap for a given level of chest object
 */
static int pick_one_chest_trap(int level)
{
	int count = 0, pick;
	struct chest_trap *trap;

	/* Count possible traps (starting after the "locked" trap) */
	for (trap = chest_traps->next; trap; trap = trap->next) {
		if (trap->level <= level) count++;
	}

	/* Pick a trap, return the pval */
	pick = randint0(count);
	for (trap = chest_traps->next; trap; trap = trap->next) {
		if (!pick--) break;
	}
	return trap->pval;
}

/**
 * Pick a set of chest traps
 * Currently this only depends on the level of the chest object
 */
int pick_chest_traps(struct object *obj)
{
	int level = obj->kind->level;
	int trap = 0;

	/* One in ten chance of no trap */
	if (one_in_(10)) {
		return 1;
	}

	/* Pick a trap, add it */
	trap |= pick_one_chest_trap(level);

	/* Level dependent chance of a second trap (may overlap the first one) */
	if ((level > 5) && one_in_(1 + ((65 - level) / 10))) {
		trap |= pick_one_chest_trap(level);
	}

	/* Chance of a third trap for deep chests (may overlap existing traps) */
	if ((level > 45) && one_in_(65 - level)) {
		trap |= pick_one_chest_trap(level);
		/* Small chance of a fourth trap (may overlap existing traps) */
		if (one_in_(40)) {
			trap |= pick_one_chest_trap(level);
		}
	}

	return trap;
}

/**
 * Unlock a chest
 */
void unlock_chest(struct object *obj)
{
	obj->pval = (0 - obj->pval);
}

/**
 * Determine if a grid contains a chest matching the query type, and
 * return a pointer to the first such chest
 */
struct object *chest_check(struct loc grid, enum chest_query check_type)
{
	struct object *obj;

	/* Scan all objects in the grid */
	for (obj = square_object(cave, grid); obj; obj = obj->next) {
		/* Ignore if requested */
		if (ignore_item_ok(obj)) continue;

		/* Check for chests */
		switch (check_type) {
		case CHEST_ANY:
			if (tval_is_chest(obj))
				return obj;
			break;
		case CHEST_OPENABLE:
			if (tval_is_chest(obj) && (obj->pval != 0))
				return obj;
			break;
		case CHEST_TRAPPED:
			if (is_trapped_chest(obj) && obj->known && obj->known->pval)
				return obj;
			break;
		}
	}

	/* No chest */
	return NULL;
}


/**
 * Return the number of grids holding a chests around (or under) the character.
 * If requested, count only trapped chests.
 */
int count_chests(struct loc *grid, enum chest_query check_type)
{
	int d, count;

	/* Count how many matches */
	count = 0;

	/* Check around (and under) the character */
	for (d = 0; d < 9; d++) {
		/* Extract adjacent (legal) location */
		struct loc grid1 = loc_sum(player->grid, ddgrid_ddd[d]);

		/* No (visible) chest is there */
		if (!chest_check(grid1, check_type)) continue;

		/* Count it */
		++count;

		/* Remember the location of the last chest found */
		*grid = grid1;
	}

	/* All done */
	return count;
}


/**
 * Allocate objects upon opening a chest
 *
 * Disperse treasures from the given chest, centered at (x,y).
 *
 * Wooden chests contain 1 item, Iron chests contain 2 items,
 * and Steel chests contain 3 items.  Small chests now contain good items,
 * large chests great items, out of depth for the level on which the chest
 * is generated.
 *
 * Judgment of size and construction of chests is currently made from the name.
 */
static void chest_death(struct loc grid, struct object *chest)
{
	int number, level;
	bool large = strstr(chest->kind->name, "Large") ? true : false;;

	/* Zero pval means empty chest */
	if (!chest->pval)
		return;

	/* Determine how much to drop (see above) */
	if (strstr(chest->kind->name, "wooden")) {
		number = 1;
	} else if (strstr(chest->kind->name, "iron")) {
		number = 2;
	} else if (strstr(chest->kind->name, "steel")) {
		number = 3;
	} else {
		number = randint1(3);
	}

	/* Drop some valuable objects (non-chests) */
	level = chest->origin_depth + 5;
	while (number > 0) {
		struct object *treasure;
		treasure = make_object(cave, level, true, large, false, NULL, 0);
		if (!treasure)
			continue;
		if (tval_is_chest(treasure)) {
			object_delete(&treasure);
			continue;
		}

		treasure->origin = ORIGIN_CHEST;
		treasure->origin_depth = chest->origin_depth;
		drop_near(cave, &treasure, 0, grid, true, false);
		number--;
	}

	/* Chest is now empty */
	chest->pval = 0;
	chest->known->pval = 0;
}


/**
 * Chests have traps too.
 */
static void chest_trap(struct object *obj)
{
	int traps = obj->pval;
	struct chest_trap *trap;
	bool ident;

	/* Ignore disarmed chests */
	if (traps <= 0) return;

	/* Apply trap effects */
	for (trap = chest_traps; trap; trap = trap->next) {
		if (trap->pval & traps) {
			if (trap->msg) {
				msg(trap->msg);
			}
			if (trap->effect) {
				effect_do(trap->effect, source_chest_trap(trap), obj, &ident,
						  false, 0, 0, 0, NULL);
			}
			if (trap->destroy) {
				obj->pval = 0;
				break;
			}
		}
	}
}


/**
 * Attempt to open the given chest at the given location
 *
 * Assume there is no monster blocking the destination
 *
 * Returns true if repeated commands may continue
 */
bool do_cmd_open_chest(struct loc grid, struct object *obj)
{
	int i, j;

	bool flag = true;

	bool more = false;

	/* Attempt to unlock it */
	if (obj->pval > 0) {
		/* Assume locked, and thus not open */
		flag = false;

		/* Get the "disarm" factor */
		i = player->state.skills[SKILL_DISARM_PHYS];

		/* Penalize some conditions */
		if (player->timed[TMD_BLIND] || no_light()) i = i / 10;
		if (player->timed[TMD_CONFUSED] || player->timed[TMD_IMAGE]) i = i / 10;

		/* Extract the difficulty */
		j = i - obj->pval;

		/* Always have a small chance of success */
		if (j < 2) j = 2;

		/* Success -- May still have traps */
		if (randint0(100) < j) {
			msgt(MSG_LOCKPICK, "You have picked the lock.");
			player_exp_gain(player, 1);
			flag = true;
		} else {
			/* We may continue repeating */
			more = true;
			event_signal(EVENT_INPUT_FLUSH);
			msgt(MSG_LOCKPICK_FAIL, "You failed to pick the lock.");
		}
	}

	/* Allowed to open */
	if (flag) {
		/* Apply chest traps, if any and player is not trapsafe */
		if (!player_is_trapsafe(player)) {
			chest_trap(obj);
		} else if ((obj->pval > 0) && player_of_has(player, OF_TRAP_IMMUNE)) {
			/* Learn trap immunity if there are traps */
			equip_learn_flag(player, OF_TRAP_IMMUNE);
		}

		/* Let the Chest drop items */
		chest_death(grid, obj);

		/* Ignore chest if autoignore calls for it */
		player->upkeep->notice |= PN_IGNORE;
	}

	/* Empty chests were always ignored in ignore_item_okay so we
	 * might as well ignore it here
	 */
	if (obj->pval == 0)
		obj->known->notice |= OBJ_NOTICE_IGNORE;

	/* Redraw chest, to be on the safe side (it may have been ignored) */
	square_light_spot(cave, grid);

	/* Result */
	return (more);
}


/**
 * Attempt to disarm the chest at the given location
 * Assume there is no monster blocking the destination
 *
 * The calculation of difficulty assumes that there are 6 types of chest
 * trap; if more are added, it will need adjusting.
 *
 * Returns true if repeated commands may continue
 */
bool do_cmd_disarm_chest(struct object *obj)
{
	int skill = player->state.skills[SKILL_DISARM_PHYS], diff;
	struct chest_trap *traps;
	bool physical = false;
	bool magic = false;
	bool more = false;

	/* Check whether the traps are magic, physical or both */
	for (traps = chest_traps; traps; traps = traps->next) {
		if (!(traps->pval & obj->pval)) continue;
		if (traps->magic) {
			magic = true;
		} else {
			physical = true;
		}
	}

	/* Physical disarming is the default, if there are magic traps we adjust */ 
	if (magic) {
		if (physical) {
			skill = (player->state.skills[SKILL_DISARM_MAGIC] +
					 player->state.skills[SKILL_DISARM_PHYS]) / 2;
		} else {
			skill = player->state.skills[SKILL_DISARM_MAGIC];
		}
	}

	/* Penalize some conditions */
	if (player->timed[TMD_BLIND] || no_light()) {
		skill /= 10;
	}
	if (player->timed[TMD_CONFUSED] || player->timed[TMD_IMAGE]) {
		skill /= 10;
	}

	/* Extract the difficulty */
	diff = skill - obj->pval;

	/* Always have a small chance of success */
	if (diff < 2) diff = 2;

	/* Must find the trap first. */
	if (!obj->known->pval) {
		msg("I don't see any traps.");
	} else if (!is_trapped_chest(obj)) {
		/* Already disarmed/unlocked or no traps */
		msg("The chest is not trapped.");
	} else if (randint0(100) < diff) {
		/* Success (get a lot of experience) */
		msgt(MSG_DISARM, "You have disarmed the chest.");
		player_exp_gain(player, obj->pval);
		obj->pval = (0 - obj->pval);
	} else if (randint0(100) < diff) {
		/* Failure -- Keep trying */
		more = true;
		event_signal(EVENT_INPUT_FLUSH);
		msg("You failed to disarm the chest.");
	} else {
		/* Failure -- Set off the trap */
		msg("You set off a trap!");
		chest_trap(obj);
	}

	/* Result */
	return (more);
}
