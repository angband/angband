/**
 * \file mon-timed.c
 * \brief Monster timed effects.
 *
 * Copyright (c) 1997-2007 Ben Harrison, James E. Wilson, Robert A. Koeneke
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
#include "mon-desc.h"
#include "mon-lore.h"
#include "mon-msg.h"
#include "mon-predicate.h"
#include "mon-spell.h"
#include "mon-timed.h"
#include "mon-util.h"
#include "player-calcs.h"

/**
 * The different ways increases can stack - see mon_inc_timed()
 */
enum stack_type {
	STACK_NO,
	STACK_INCR,
	STACK_MAX
};

/**
 * Monster timed effects.
 */
static struct mon_timed_effect {
	const char *name;
	bool gets_save;
	enum stack_type stacking;
	int flag_resist;
	int max_timer;
	int message_begin;
	int message_end;
	int message_increase;
} effects[] = {
	#define MON_TMD(a, b, c, d, e, f, g, h) { #a, b, STACK_##c, d, e, f, g, h },
	#include "list-mon-timed.h"
	#undef MON_TMD
};

/**
 * Find the timed monster effect with the name `name`.
 *
 * Returns -1 on failure.
 */
int mon_timed_name_to_idx(const char *name)
{
    for (size_t i = 0; i < MON_TMD_MAX; i++) {
        if (streq(name, effects[i].name))
            return i;
    }

    return -1;
}

/**
 * Roll the saving throw for monsters resisting a timed effect.
 */
static bool saving_throw(const struct monster *mon, int effect_type, int timer, int flag)
{
	int resist_chance = MIN(
								90,
								mon->race->level + MAX(0, 25 - timer / 2)
						   );

	/* Give unique monsters a double check */
	if (rf_has(mon->race->flags, RF_UNIQUE) &&
			(randint0(100) < resist_chance)) {
		return true;
	}

	return randint0(100) < resist_chance;
}

/**
 * Determines whether the given monster successfully resists the given effect.
 */
static bool does_resist(const struct monster *mon, int effect_type, int timer, int flag)
{
	assert(mon != NULL);
	assert(effect_type >= 0);
	assert(effect_type < MON_TMD_MAX);

	struct mon_timed_effect *effect = &effects[effect_type];
	struct monster_lore *lore = get_lore(mon->race);

	/* Sometimes the game can override the monster's innate resistance */
	if (flag & MON_TMD_FLG_NOFAIL) {
		return false;
	}

	/* Check resistances from monster flags */
	if (rf_has(mon->race->flags, effect->flag_resist)) {
		lore_learn_flag_if_visible(lore, mon, effect->flag_resist);
		return true;
	}

	/* Some effects get a saving throw; others don't */
	if (effect->gets_save == true) {
		return saving_throw(mon, effect_type, timer, flag);
	} else {
		return false;
	}
}

/**
 * Attempts to set the timer of the given monster effect to `timer`.
 *
 * Checks to see if the monster resists the effect, using does_resist().
 * If not, the effect is set to `timer` turns. If `timer` is 0, or if the
 * effect timer was 0, or if MON_TMD_FLG_NOTIFY is set in `flag`, then a
 * message is printed, unless MON_TMD_FLG_NOMESSAGE is set in `flag`.
 *
 * Set a timed monster event to 'v'.  Give messages if the right flags are set.
 * Check if the monster is able to resist the spell.  Mark the lore.
 *
 * Returns true if the monster was affected, false if not.
 */
static bool mon_set_timed(struct monster *mon,
		int effect_type,
		int timer,
		int flag,
		bool id)
{
	assert(mon != NULL);
	assert(mon->race != NULL);
	assert(effect_type >= 0);
	assert(effect_type < MON_TMD_MAX);
	assert(timer >= 0);

	struct mon_timed_effect *effect = &effects[effect_type];

	bool check_resist;
	bool resisted = false;

	int m_note = 0;
	int old_timer = mon->m_timed[effect_type];

	/* Limit time of effect */
	if (timer > effect->max_timer) {
		timer = effect->max_timer;
	}

	/* No change */
	if (old_timer == timer) {
		return false;
	} else if (timer == 0) {
		/* Turning off, usually mention */
		m_note = effect->message_end;
		flag |= MON_TMD_FLG_NOTIFY;
		check_resist = false;
	} else if (old_timer == 0) {
		/* Turning on, usually mention */
		m_note = effect->message_begin;
		flag |= MON_TMD_FLG_NOTIFY;
		check_resist = true;
	} else if (timer > old_timer) {
		/* Different message for increases, but don't automatically mention. */
		m_note = effect->message_increase;
		check_resist = true;
	} else {
		/* Decreases don't get a message, but never resist them */
		check_resist = false;
	}

	/* Determine if the monster resisted or not, if appropriate */
	if (check_resist && does_resist(mon, effect_type, timer, flag)) {
		resisted = true;
		m_note = MON_MSG_UNAFFECTED;
	} else {
		mon->m_timed[effect_type] = timer;

		if (player->upkeep->health_who == mon)
			player->upkeep->redraw |= (PR_HEALTH);

		/* Update the visuals, as appropriate. */
		player->upkeep->redraw |= (PR_MONLIST);
	}

	/* Print a message if there is one, if the effect allows for it, and if
	 * either the monster is visible, or we're trying to ID something */
	if (m_note &&
		!(flag & MON_TMD_FLG_NOMESSAGE) &&
		(flag & MON_TMD_FLG_NOTIFY)
		&& monster_is_obvious(mon)) {
			char m_name[80];
			monster_desc(m_name, sizeof(m_name), mon, MDESC_IND_HID);
			add_monster_message(mon, m_note, true);
	}

	return !resisted;
}

/** Minimum number of turns a new timed effect can last */
#define MON_INC_MIN_TURNS		2

/**
 * Increases the timed effect `effect_type` by `timer`.
 *
 * Calculates the new timer, then passes that to mon_set_timed().
 * Note that each effect has a maximum number of turns it can be active for.
 * If this function would put an effect timer over that cap, it sets it for
 * that cap instead.
 *
 * Returns true if the monster's timer changed.
 */
bool mon_inc_timed(struct monster *mon, int effect_type, int timer, int flag,
				   bool id)
{
	assert(effect_type >= 0);
	assert(effect_type < MON_TMD_MAX);
	assert(timer > 0); /* For negative amounts, we use mon_dec_timed instead */

	struct mon_timed_effect *effect = &effects[effect_type];
	int new_value = timer;

	/* Make it last for a mimimum # of turns if it is a new effect */
	if (mon->m_timed[effect_type] == 0 && timer < MON_INC_MIN_TURNS) {
		timer = MON_INC_MIN_TURNS;
	}

	/* Stack effects correctly */
	switch (effect->stacking) {
		case STACK_NO: {
			new_value = mon->m_timed[effect_type];
			if (new_value == 0) {
				new_value = timer;
			}
			break;
		}

		case STACK_MAX: {
			new_value = MAX(mon->m_timed[effect_type], timer);
			break;
		}

		case STACK_INCR: {
			new_value = mon->m_timed[effect_type] + timer;
			break;
		}
	}

	return mon_set_timed(mon, effect_type, new_value, flag, id);
}

/**
 * Decreases the timed effect `effect_type` by `timer`.
 *
 * Calculates the new timer, then passes that to mon_set_timed().
 * If a timer would be set to a negative number, it is set to 0 instead.
 * Note that decreasing a timed effect should never fail.
 *
 * Returns true if the monster's timer changed.
 */
bool mon_dec_timed(struct monster *mon, int effect_type, int timer, int flag,
				   bool id)
{
	assert(effect_type >= 0);
	assert(effect_type < MON_TMD_MAX);
	assert(timer > 0); /* For negative amounts, we use mon_inc_timed instead */

	int new_level = mon->m_timed[effect_type] - timer;
	if (new_level < 0) {
		new_level = 0;
	}

	return mon_set_timed(mon, effect_type, new_level, flag, id);
}

/**
 * Clears the timed effect `effect_type`.
 *
 * Returns true if the monster's timer was changed.
 */
bool mon_clear_timed(struct monster *mon, int effect_type, int flag, bool id)
{
	assert(effect_type >= 0);
	assert(effect_type < MON_TMD_MAX);

	if (mon->m_timed[effect_type] == 0) {
		return false;
	} else {
		return mon_set_timed(mon, effect_type, 0, flag, id);
	}
}

/**
 * The level at which an effect is affecting a monster.
 * Levels range from 0 (unaffected) to 5 (maximum effect).
 */
int monster_effect_level(struct monster *mon, int effect_type)
{
	struct mon_timed_effect *effect = &effects[effect_type];
	int divisor = MAX(effect->max_timer / 5, 1);
	return MIN((mon->m_timed[effect_type] + divisor - 1) / divisor, 5);
}
