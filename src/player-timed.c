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

int PY_FOOD_MAX;
int PY_FOOD_FULL;
int PY_FOOD_HUNGRY;
int PY_FOOD_WEAK;
int PY_FOOD_FAINT;
int PY_FOOD_STARVE;

/**
 * ------------------------------------------------------------------------
 * Parsing functions for player_timed.txt
 * ------------------------------------------------------------------------ */

const char *list_player_flag_names[] = {
	#define PF(a) #a,
	#include "list-player-flags.h"
	#undef PF
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

	/* Name may be a dummy (eg hunger)*/
	if (strlen(l->name) == 1) {
		string_free(l->name);
		l->name = NULL;
	}

	l->up_msg = string_make(parser_getsym(p, "up_msg"));

	/* Message may be a dummy */
	if (strlen(l->up_msg) == 1) {
		string_free(l->up_msg);
		l->up_msg = NULL;
	}

	if (parser_hasval(p, "down_msg")) {
		l->down_msg = string_make(parser_getsym(p, "down_msg"));
	}

	/* Set food constants and deal with percentages */
	if (streq(t->name, "FOOD")) {
		l->max *= z_info->food_value;
		if (streq(l->name, "Starving")) {
			PY_FOOD_STARVE = l->max;
		} else if (streq(l->name, "Faint")) {
			PY_FOOD_FAINT = l->max;
		} else if (streq(l->name, "Weak")) {
			PY_FOOD_WEAK = l->max;
		} else if (streq(l->name, "Hungry")) {
			PY_FOOD_HUNGRY = l->max;
		} else if (streq(l->name, "Fed")) {
			PY_FOOD_FULL = l->max;
		} else if (streq(l->name, "Full")) {
			PY_FOOD_MAX = l->max;
		}
	}

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
	parser_reg(p, "grade sym color int max sym name sym up_msg ?sym down_msg", parse_player_timed_grade);
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
			if (grade->up_msg) string_free(grade->up_msg);
			if (grade->down_msg) string_free(grade->down_msg);
			mem_free(grade);
			grade = next;
		}
		effect->grade = NULL;

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
 * Swap stats at random to temporarily scramble the player's stats.
 */
static void player_scramble_stats(struct player *p)
{
	int max1, cur1, max2, cur2, i, j, swap;

	/* Fisher-Yates shuffling algorithm */
	for (i = STAT_MAX - 1; i > 0; --i) {
		j = randint0(i);

		max1 = p->stat_max[i];
		cur1 = p->stat_cur[i];
		max2 = p->stat_max[j];
		cur2 = p->stat_cur[j];

		p->stat_max[i] = max2;
		p->stat_cur[i] = cur2;
		p->stat_max[j] = max1;
		p->stat_cur[j] = cur1;

		/* Record what we did */
		swap = p->stat_map[i];
		p->stat_map[i] = p->stat_map[j];
		p->stat_map[j] = swap;
	}

	return;
}

/**
 * Undo scrambled stats when effect runs out.
 */
static void player_fix_scramble(struct player *p)
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
 * Return true if the player timed effect matches the given string
 */
bool player_timed_grade_eq(struct player *p, int idx, char *match)
{
	if (p->timed[idx]) {
		struct timed_grade *grade = timed_effects[idx].grade;
		while (p->timed[idx] > grade->max) {
			grade = grade->next;
		}
		if (grade->name && streq(grade->name, match)) return true;
	}

	return false;
}

static bool player_of_has_prot_conf(struct player *p)
{
    bitflag collect_f[OF_SIZE], f[OF_SIZE];
    int i;

    player_flags(p, collect_f);

    for (i = 0; i < p->body.count; i++) {
        struct object *obj = slot_object(p, i);

        if (!obj) continue;
        object_flags(obj, f);
        of_union(collect_f, f);
    }

    return of_has(collect_f, OF_PROT_CONF);
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
	v = MAX(v, (idx == TMD_FOOD) ? 1 : 0);

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
	} else if (idx == TMD_OPP_CONF && player_of_has_prot_conf(p)) {
		notify = false;
	}

	/* Always mention going up a grade, otherwise on request */
	if (new_grade->grade > current_grade->grade) {
		print_custom_message(weapon, new_grade->up_msg, effect->msgt);
		notify = true;
	} else if ((new_grade->grade < current_grade->grade) &&
			   (new_grade->down_msg)) {
		print_custom_message(weapon, new_grade->down_msg, effect->msgt);
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

	/* Handle stat swap */
	if (idx == TMD_SCRAMBLE) {
		if (p->timed[idx] == 0) {
			player_scramble_stats(p);
		} else if (v == 0) {
			player_fix_scramble(p);
		}
	}

	/* Use the value */
	p->timed[idx] = v;

	/* Sort out the sprint effect */
	if (idx == TMD_SPRINT && v == 0) {
		player_inc_timed(p, TMD_SLOW, 100, true, false);
	}

	if (notify) {
		/* Disturb */
		disturb(p);

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

