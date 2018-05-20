/**
 * \file player-timed.c
 * \brief Timed effects handling
 *
 * Copyright (c) 1997 Ben Harrison
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
#include "datafile.h"
#include "init.h"
#include "mon-util.h"
#include "obj-gear.h"
#include "obj-knowledge.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "player-util.h"

/**
 * ------------------------------------------------------------------------
 * Parsing functions for player_timed.txt
 * ------------------------------------------------------------------------ */

const char *list_player_flag_names[] = {
#define PF(a, b, c) #a,
	#include "list-player-flags.h"
	#undef ELEM
	NULL
};

struct timed_effect_data timed_effects[TMD_MAX] = {
	#define TMD(a, b, c)	{ #a, b, c },
	#include "list-player-timed.h"
	#undef TMD
};

int timed_name_to_idx(const char *name)
{
    for (size_t i = 0; i < N_ELEMENTS(timed_effects); i++) {
        if (my_stricmp(name, timed_effects[i].name) == 0) {
            return i;
        }
    }

    return -1;
}

/**
 * List of timed effect names
 */
static const char *list_timed_effect_names[] = {
	#define TMD(a, b, c) #a,
	#include "list-player-timed.h"
	#undef TMD
	"MAX",
	NULL
};

static enum parser_error parse_player_timed_name(struct parser *p)
{
	const char *name = parser_getstr(p, "name");
	int index;

	if (grab_name("timed effect",
			name,
			list_timed_effect_names,
			N_ELEMENTS(list_timed_effect_names),
			&index)) {
		/* XXX not a desctiptive error */
		return PARSE_ERROR_INVALID_SPELL_NAME;
	}

	struct timed_effect_data *t = &timed_effects[index];

	t->index = index;
	parser_setpriv(p, t);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_desc(struct parser *p)
{
	struct timed_effect_data *t = parser_priv(p);
	assert(t);

	t->desc = string_append(t->desc, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_end_message(struct parser *p)
{
	struct timed_effect_data *t = parser_priv(p);
	assert(t);

	t->on_end = string_append(t->on_end, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_increase_message(struct parser *p)
{
	struct timed_effect_data *t = parser_priv(p);
	assert(t);

	t->on_increase = string_append(t->on_increase, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_decrease_message(struct parser *p)
{
	struct timed_effect_data *t = parser_priv(p);
	assert(t);

	t->on_decrease = string_append(t->on_decrease, parser_getstr(p, "text"));
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_message_type(struct parser *p)
{
	struct timed_effect_data *t = parser_priv(p);
	assert(t);

	t->msgt = message_lookup_by_name(parser_getsym(p, "type"));

	return t->msgt < 0 ?
				PARSE_ERROR_INVALID_MESSAGE :
				PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_fail(struct parser *p)
{
	struct timed_effect_data *t = parser_priv(p);
	assert(t);

	t->fail_code = parser_getuint(p, "code");

	const char *name = parser_getstr(p, "flag");
	if (t->fail_code == TMD_FAIL_FLAG_OBJECT) {
		int flag = lookup_flag(list_obj_flag_names, name);
		if (flag == FLAG_END)
			return PARSE_ERROR_INVALID_FLAG;
		else
			t->fail = flag;
	} else if (t->fail_code == TMD_FAIL_FLAG_PLAYER) {
		int flag = lookup_flag(list_player_flag_names, name);
		if (flag == FLAG_END)
			return PARSE_ERROR_INVALID_FLAG;
		else
			t->fail = flag;
	} else if ((t->fail_code == TMD_FAIL_FLAG_RESIST) ||
			   (t->fail_code == TMD_FAIL_FLAG_VULN)) {
		size_t i = 0;
		while (list_element_names[i] && !streq(list_element_names[i], name))
			i++;

		if (i == ELEM_MAX)
			return PARSE_ERROR_INVALID_FLAG;
		else
			t->fail = i;
	} else {
		return PARSE_ERROR_INVALID_FLAG;
	}

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_player_timed_grade(struct parser *p)
{
	struct timed_effect_data *t = parser_priv(p);
	struct timed_grade *current = t->grade;
	struct timed_grade *l = mem_zalloc(sizeof(*l));
    const char *color = parser_getsym(p, "color");
    int attr = 0;
	assert(t);

	/* Make a zero grade structure if there isn't one */
	if (!current) {
		t->grade = mem_zalloc(sizeof(struct timed_grade));
		current = t->grade;
	}

	/* Move to the highest grade so far */
	while (current->next) {
		current = current->next;
	}

	/* Add the new one */
	current->next = l;
	l->grade = current->grade + 1;

    if (strlen(color) > 1) {
		attr = color_text_to_attr(color);
    } else {
		attr = color_char_to_attr(color[0]);
	}
    if (attr < 0)
		return PARSE_ERROR_INVALID_COLOR;
    l->color = attr;

	l->max = parser_getint(p, "max");
	l->name = string_make(parser_getsym(p, "name"));
	l->msg = string_make(parser_getsym(p, "msg"));
	return PARSE_ERROR_NONE;
}

static struct parser *init_parse_player_timed(void)
{
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);
	parser_reg(p, "name str name", parse_player_timed_name);
	parser_reg(p, "desc str text", parse_player_timed_desc);
	parser_reg(p, "on-end str text", parse_player_timed_end_message);
	parser_reg(p, "on-increase str text", parse_player_timed_increase_message);
	parser_reg(p, "on-decrease str text", parse_player_timed_decrease_message);
	parser_reg(p, "msgt sym type", parse_player_timed_message_type);
	parser_reg(p, "fail uint code str flag", parse_player_timed_fail);
	parser_reg(p, "grade sym color int max sym name sym msg", parse_player_timed_grade);
	return p;
}

static errr run_parse_player_timed(struct parser *p)
{
	return parse_file_quit_not_found(p, "player_timed");
}

static errr finish_parse_player_timed(struct parser *p)
{
	parser_destroy(p);
	return 0;
}

static void cleanup_player_timed(void)
{
	for (size_t i = 0; i < TMD_MAX; i++) {
		struct timed_effect_data *effect = &timed_effects[i];
			struct timed_grade *grade = effect->grade;

		while (grade) {
			struct timed_grade *next = grade->next;
			string_free(grade->name);
			string_free(grade->msg);
			mem_free(grade);
			grade = next;
		}

		string_free(effect->desc);

		if (effect->on_end)
			string_free(effect->on_end);
		if (effect->on_increase)
			string_free(effect->on_increase);
		if (effect->on_decrease)
			string_free(effect->on_decrease);

		effect->desc        = NULL;
		effect->on_end      = NULL;
		effect->on_increase = NULL;
		effect->on_decrease = NULL;
	}
}

struct file_parser player_timed_parser = {
	"player timed effects",
	init_parse_player_timed,
	run_parse_player_timed,
	finish_parse_player_timed,
	cleanup_player_timed
};


/**
 * ------------------------------------------------------------------------
 * Utilities for more complex or anomolous effects
 * ------------------------------------------------------------------------ */
/**
 * Undo scrambled stats when effect runs out.
 */
void player_fix_scramble(struct player *p)
{
	/* Figure out what stats should be */
	int new_cur[STAT_MAX];
	int new_max[STAT_MAX];

	for (int i = 0; i < STAT_MAX; i++) {
		new_cur[p->stat_map[i]] = p->stat_cur[i];
		new_max[p->stat_map[i]] = p->stat_max[i];
	}

	/* Apply new stats and clear the stat_map */
	for (int i = 0; i < STAT_MAX; i++) {
		p->stat_cur[i] = new_cur[i];
		p->stat_max[i] = new_max[i];
		p->stat_map[i] = i;
	}
}

/**
 * Set "player->food", notice observable changes
 *
 * The "player->food" variable can get as large as 20000, allowing the
 * addition of the most "filling" item, Elvish Waybread, which adds
 * 7500 food units, without overflowing the 32767 maximum limit.
 *
 * Perhaps we should disturb the player with various messages,
 * especially messages about hunger status changes.  XXX XXX XXX
 *
 * Digestion of food is handled in "dungeon.c", in which, normally,
 * the player digests about 20 food units per 100 game turns, more
 * when "fast", more when "regenerating", less with "slow digestion".
 */
bool player_set_food(struct player *p, int v)
{
	int old_aux, new_aux;

	bool notice = false;

	/* Hack -- Force good values */
	v = MIN(v, PY_FOOD_MAX);
	v = MAX(v, 0);

	/* Current value */
	if (p->food < PY_FOOD_FAINT)      old_aux = 0;
	else if (p->food < PY_FOOD_WEAK)  old_aux = 1;
	else if (p->food < PY_FOOD_ALERT) old_aux = 2;
	else if (p->food < PY_FOOD_FULL)  old_aux = 3;
	else                              old_aux = 4;

	/* New value */
	if (v < PY_FOOD_FAINT)      new_aux = 0;
	else if (v < PY_FOOD_WEAK)  new_aux = 1;
	else if (v < PY_FOOD_ALERT) new_aux = 2;
	else if (v < PY_FOOD_FULL)  new_aux = 3;
	else                        new_aux = 4;

	/* Food increase or decrease */
	if (new_aux > old_aux) {
		switch (new_aux) {
			case 1:
				msg("You are still weak.");
				break;
			case 2:
				msg("You are still hungry.");
				break;
			case 3:
				msg("You are no longer hungry.");
				break;
			case 4:
				msg("You are full!");
				break;
		}

		/* Change */
		notice = true;
	} else if (new_aux < old_aux) {
		switch (new_aux) {
			case 0:
				msgt(MSG_NOTICE, "You are getting faint from hunger!");
				break;
			case 1:
				msgt(MSG_NOTICE, "You are getting weak from hunger!");
				break;
			case 2:
				msgt(MSG_HUNGRY, "You are getting hungry.");
				break;
			case 3:
				msgt(MSG_NOTICE, "You are no longer full.");
				break;
		}

		/* Change */
		notice = true;
	}

	/* Use the value */
	p->food = v;

	/* Nothing to notice */
	if (!notice) return (false);

	/* Disturb and update */
	disturb(player, 0);
	p->upkeep->update |= (PU_BONUS);
	p->upkeep->redraw |= (PR_STATUS);
	handle_stuff(player);

	/* Result */
	return (true);
}

/**
 * ------------------------------------------------------------------------
 * Setting, increasing, decreasing and clearing timed effects
 * ------------------------------------------------------------------------ */
/**
 * Set a timed effect.
 */
bool player_set_timed(struct player *p, int idx, int v, bool notify)
{
	assert(idx >= 0);
	assert(idx < TMD_MAX);

	struct timed_effect_data *effect = &timed_effects[idx];
	struct timed_grade *new_grade = effect->grade;
	struct timed_grade *current_grade = effect->grade;
	struct object *weapon = equipped_item_by_slot_name(p, "weapon");

	/* Lower bound */
	v = MAX(v, 0);

	/* No change */
	if (p->timed[idx] == v) {
		return false;
	}

	/* Find the grade we will be going to, and the current one */
	while (v > new_grade->max) {
		new_grade = new_grade->next;
		if (!new_grade->next) break;
	}
	while (p->timed[idx] > current_grade->max) {
		current_grade = current_grade->next;
		if (!current_grade->next) break;
	}

	/* Upper bound */
	v = MIN(v, new_grade->max);

	/* Don't mention effects which already match the player state. */
	if (idx == TMD_OPP_ACID && player_is_immune(p, ELEM_ACID)) {
		notify = false;
	} else if (idx == TMD_OPP_ELEC && player_is_immune(p, ELEM_ELEC)) {
		notify = false;
	} else if (idx == TMD_OPP_FIRE && player_is_immune(p, ELEM_FIRE)) {
		notify = false;
	} else if (idx == TMD_OPP_COLD && player_is_immune(p, ELEM_COLD)) {
		notify = false;
	} else if (idx == TMD_OPP_CONF && player_of_has(p, OF_PROT_CONF)) {
		notify = false;
	}

	/* Always mention going up a grade, otherwise on request */
	if (new_grade->grade > current_grade->grade) {
		print_custom_message(weapon, new_grade->msg, effect->msgt);
		notify = true;
	} else if (notify) {
		if (v == 0) {
			/* Finishing */
			print_custom_message(weapon, effect->on_end, MSG_RECOVER);
		} else if (p->timed[idx] > v && effect->on_decrease) {
			/* Decrementing */
			print_custom_message(weapon, effect->on_decrease, effect->msgt);
		} else if (v > p->timed[idx] && effect->on_increase) {
			/* Incrementing */
			print_custom_message(weapon, effect->on_increase, effect->msgt);
		}
	}

	/* Use the value */
	p->timed[idx] = v;

	/* Sort out the sprint effect */
	if (idx == TMD_SPRINT && v == 0) {
		player_inc_timed(p, TMD_SLOW, 100, true, false);
	}

	/* Undo stat swap */
	if (idx == TMD_SCRAMBLE && v == 0) {
		player_fix_scramble(p);
	}

	if (notify) {
		/* Disturb */
		disturb(p, 0);

		/* Update the visuals, as appropriate. */
		p->upkeep->update |= effect->flag_update;
		p->upkeep->redraw |= (PR_STATUS | effect->flag_redraw);

		/* Handle stuff */
		handle_stuff(p);
	}

	return notify;
}

/**
 * Check whether a timed effect will affect the player
 */
bool player_inc_check(struct player *p, int idx, bool lore)
{
	struct timed_effect_data *effect = &timed_effects[idx];

	/* Check that @ can be affected by this effect */
	if (!effect->fail_code) {
		return true;
	}

	/* If we're only doing this for monster lore purposes */
	if (lore) {
		if (((effect->fail_code == TMD_FAIL_FLAG_OBJECT) &&
			 (of_has(p->known_state.flags, effect->fail))) ||
			((effect->fail_code == TMD_FAIL_FLAG_RESIST) &&
			 (p->known_state.el_info[effect->fail].res_level > 0)) ||
			((effect->fail_code == TMD_FAIL_FLAG_VULN) &&
			 (p->known_state.el_info[effect->fail].res_level < 0))) {
			return false;
		} else {
			return true;
		}
	}

	/* Determine whether an effect can be prevented by a flag */
	if (effect->fail_code == TMD_FAIL_FLAG_OBJECT) {
		/* If the effect is from a monster action, extra stuff happens */
		struct monster *mon = cave->mon_current > 0 ?
				cave_monster(cave, cave->mon_current) : NULL;

		/* Effect is inhibited by an object flag */
		equip_learn_flag(p, effect->fail);

		if (mon) {
			update_smart_learn(mon, player, effect->fail, 0, -1);
		}

		if (player_of_has(p, effect->fail)) {
			if (mon) {
				msg("You resist the effect!");
			}
			return false;
		}
	} else if (effect->fail_code == TMD_FAIL_FLAG_RESIST) {
		/* Effect is inhibited by a resist */
		equip_learn_element(p, effect->fail);
		if (p->state.el_info[effect->fail].res_level > 0) {
			return false;
		}
	} else if (effect->fail_code == TMD_FAIL_FLAG_VULN) {
		/* Effect is inhibited by a vulnerability
		 * the asymmetry with resists is OK for now - NRM */
		equip_learn_element(p, effect->fail);
		if (p->state.el_info[effect->fail].res_level < 0) {
			return false;
		}
	} else if (effect->fail_code == TMD_FAIL_FLAG_PLAYER) {
		/* Effect is inhibited by a player flag */
		if (player_has(p, effect->fail)) {
			return false;
		}
	}

	/* Special cases */
	if (effect->index == TMD_POISONED && p->timed[TMD_OPP_POIS])
		return false;

	return true;
}

/**
 * Increase the timed effect `idx` by `v`.  Mention this if `notify` is true.
 * Check for resistance to the effect if `check` is true.
 */
bool player_inc_timed(struct player *p, int idx, int v, bool notify, bool check)
{
	assert(idx >= 0);
	assert(idx < TMD_MAX);

	if (check == false || player_inc_check(p, idx, false) == true) {
		/* Paralysis should be non-cumulative */
		if (idx == TMD_PARALYZED && p->timed[TMD_PARALYZED] > 0) {
			return false;
		} else {
			return player_set_timed(p,
					idx,
					p->timed[idx] + v,
					notify);
		}
	}

	return false;
}

/**
 * Decrease the timed effect `idx` by `v`.  Mention this if `notify` is true.
 */
bool player_dec_timed(struct player *p, int idx, int v, bool notify)
{
	int new_value;
	assert(idx >= 0);
	assert(idx < TMD_MAX);
	new_value = p->timed[idx] - v;

	/* Obey `notify` if not finishing; if finishing, always notify */
	if (new_value > 0) {
		return player_set_timed(p, idx, new_value, notify);
	}
	return player_set_timed(p, idx, new_value, true);
}

/**
 * Clear the timed effect `idx`.  Mention this if `notify` is true.
 */
bool player_clear_timed(struct player *p, int idx, bool notify)
{
	assert(idx >= 0);
	assert(idx < TMD_MAX);

	return player_set_timed(p, idx, 0, notify);
}

