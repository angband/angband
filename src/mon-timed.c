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
#include "mon-spell.h"
#include "mon-timed.h"
#include "mon-util.h"
#include "player-calcs.h"

/**
 * Monster timed effects.
 */
static struct mon_timed_effect {
	const char *name;
	int message_begin;
	int message_end;
	int message_increase;
	int flag_resist;
	int max_timer;
} effects[] = {
	#define MON_TMD(a, b, c, d, e, f) { #a, b, c, d, e, f },
	#include "list-mon-timed.h"
	#undef MON_TMD
};


int mon_timed_name_to_idx(const char *name)
{
    int i;
    for (i = 0; !streq(effects[i].name, "MAX"); i++) {
        if (streq(name, effects[i].name))
            return i;
    }

    return -1;
}

/**
 * Determines whether the given monster successfully resists the given effect.
 *
 * If MON_TMD_FLG_NOFAIL is set in `flag`, this returns FALSE.
 * Then we determine if the monster resists the effect for some racial
 * reason. For example, the monster might have the NO_SLEEP flag, in which
 * case it always resists sleep. Or if it breathes chaos, it always resists
 * confusion. If the given monster doesn't resist for any of these reasons,
 * then it makes a saving throw. If MON_TMD_MON_SOURCE is set in `flag`,
 * indicating that another monster caused this effect, then the chance of
 * success on the saving throw just depends on the monster's native depth.
 * Otherwise, the chance of success decreases as `timer` increases.
 *
 * Also marks the lore for any appropriate resists.
 */
static bool mon_resist_effect(const struct monster *mon, int ef_idx, int timer, u16b flag)
{
	struct mon_timed_effect *effect;
	int resist_chance;
	struct monster_lore *lore;

	assert(ef_idx >= 0 && ef_idx < MON_TMD_MAX);
	assert(mon);

	effect = &effects[ef_idx];
	lore = get_lore(mon->race);
	
	/* Hasting never fails */
	if (ef_idx == MON_TMD_FAST) return (FALSE);
	
	/* Some effects are marked to never fail */
	if (flag & MON_TMD_FLG_NOFAIL) return (FALSE);

	/* A sleeping monster resists further sleeping */
	if (ef_idx == MON_TMD_SLEEP && mon->m_timed[ef_idx]) return (TRUE);

	/* If the monster resists innately, learn about it */
	if (rf_has(mon->race->flags, effect->flag_resist)) {
		if (mflag_has(mon->mflag, MFLAG_VISIBLE))
			rf_on(lore->flags, effect->flag_resist);

		return (TRUE);
	}

	/* Monsters with specific breaths resist stunning */
	if (ef_idx == MON_TMD_STUN &&
		(rsf_has(mon->race->spell_flags, RSF_BR_SOUN) ||
		 rsf_has(mon->race->spell_flags, RSF_BR_WALL))) {
		/* Add the lore */
		if (mflag_has(mon->mflag, MFLAG_VISIBLE)) {
			if (rsf_has(mon->race->spell_flags, RSF_BR_SOUN))
				rsf_on(lore->spell_flags, RSF_BR_SOUN);
			if (rsf_has(mon->race->spell_flags, RSF_BR_WALL))
				rsf_on(lore->spell_flags, RSF_BR_WALL);
		}

		return (TRUE);
	}

	/* Monsters with specific breaths resist confusion */
	if ((ef_idx == MON_TMD_CONF) &&
		rsf_has(mon->race->spell_flags, RSF_BR_CHAO)) {
		/* Add the lore */
		if (mflag_has(mon->mflag, MFLAG_VISIBLE))
			if (rsf_has(mon->race->spell_flags, RSF_BR_CHAO))
				rsf_on(lore->spell_flags, RSF_BR_CHAO);

		return (TRUE);
	}

	/* Inertia breathers resist slowing */
	if (ef_idx == MON_TMD_SLOW && rsf_has(mon->race->spell_flags, RSF_BR_INER)){
		rsf_on(lore->spell_flags, RSF_BR_INER);
		return (TRUE);
	}

	/* Calculate the chance of the monster making its saving throw. */
	if (ef_idx == MON_TMD_SLEEP)
		timer /= 25; /* Hack - sleep uses much bigger numbers */

	if (flag & MON_TMD_MON_SOURCE)
		resist_chance = mon->race->level;
	else
		resist_chance = mon->race->level + 40 - (timer / 2);

	if (randint0(100) < resist_chance) return (TRUE);

	/* Uniques are doubly hard to affect */
	if (rf_has(mon->race->flags, RF_UNIQUE))
		if (randint0(100) < resist_chance) return (TRUE);

	return (FALSE);
}

/**
 * Attempts to set the timer of the given monster effect to `timer`.
 *
 * Checks to see if the monster resists the effect, using mon_resist_effect().
 * If not, the effect is set to `timer` turns. If `timer` is 0, or if the
 * effect timer was 0, or if MON_TMD_FLG_NOTIFY is set in `flag`, then a
 * message is printed, unless MON_TMD_FLG_NOMESSAGE is set in `flag`.
 *
 * Set a timed monster event to 'v'.  Give messages if the right flags are set.
 * Check if the monster is able to resist the spell.  Mark the lore.
 * Returns TRUE if the monster was affected.
 * Return FALSE if the monster was unaffected.
 */
static bool mon_set_timed(struct monster *mon, int ef_idx, int timer,
						  u16b flag, bool id)
{
	bool check_resist = FALSE;
	bool resisted = FALSE;

	struct mon_timed_effect *effect;

	int m_note = 0;
	int old_timer;

	assert(ef_idx >= 0 && ef_idx < MON_TMD_MAX);
	effect = &effects[ef_idx];

	assert(mon);
	assert(mon->race);

	old_timer = mon->m_timed[ef_idx];

	/* No change */
	if (old_timer == timer)
		return FALSE;

	if (timer == 0) {
		/* Turning off, usually mention */
		m_note = effect->message_end;
		flag |= MON_TMD_FLG_NOTIFY;
	} else if (old_timer == 0) {
		/* Turning on, usually mention */
		flag |= MON_TMD_FLG_NOTIFY;
		m_note = effect->message_begin;
		check_resist = TRUE;
	} else if (timer > old_timer) {
		/* Different message for increases, but don't automatically mention. */
		m_note = effect->message_increase;
		check_resist = TRUE;
	} /* Decreases don't get a message */

	/* Determine if the monster resisted or not, if appropriate */
	if (check_resist)
		resisted = mon_resist_effect(mon, ef_idx, timer, flag);

	if (resisted)
		m_note = MON_MSG_UNAFFECTED;
	else
		mon->m_timed[ef_idx] = timer;

	if (player->upkeep->health_who == mon)
		player->upkeep->redraw |= (PR_HEALTH);

	/* Update the visuals, as appropriate. */
	player->upkeep->redraw |= (PR_MONLIST);

	/* Print a message if there is one, if the effect allows for it, and if
	 * either the monster is visible, or we're trying to ID something */
	if (m_note && !(flag & MON_TMD_FLG_NOMESSAGE) && (flag & MON_TMD_FLG_NOTIFY)
		&& ((mflag_has(mon->mflag, MFLAG_VISIBLE) &&
			 !mflag_has(mon->mflag, MFLAG_UNAWARE)) || id)) {
		char m_name[80];
		monster_desc(m_name, sizeof(m_name), mon, MDESC_IND_HID);
		add_monster_message(m_name, mon, m_note, TRUE);
	}

	return !resisted;
}


/**
 * Increases the timed effect `ef_idx` by `timer`.
 *
 * Calculates the new timer, then passes that to mon_set_timed().
 * Note that each effect has a maximum number of turns it can be active for.
 * If this function would put an effect timer over that cap, it sets it for
 * that cap instead.
 *
 * Returns TRUE if the monster's timer changed.
 */
bool mon_inc_timed(struct monster *mon, int ef_idx, int timer, u16b flag,
				   bool id)
{
	struct mon_timed_effect *effect;

	assert(ef_idx >= 0 && ef_idx < MON_TMD_MAX);
	effect = &effects[ef_idx];

	/* For negative amounts, we use mon_dec_timed instead */
	assert(timer > 0);

	/* Make it last for a mimimum # of turns if it is a new effect */
	if ((!mon->m_timed[ef_idx]) && (timer < 2)) timer = 2;

	/* New counter amount - prevent overflow */
	if (MAX_SHORT - timer < mon->m_timed[ef_idx])
		timer = MAX_SHORT;
	else
		timer += mon->m_timed[ef_idx];

	/* Reduce to max_timer if necessary*/
	if (timer > effect->max_timer)
		timer = effect->max_timer;

	return mon_set_timed(mon, ef_idx, timer, flag, id);
}

/**
 * Decreases the timed effect `ef_idx` by `timer`.
 *
 * Calculates the new timer, then passes that to mon_set_timed().
 * If a timer would be set to a negative number, it is set to 0 instead.
 * Note that decreasing a timed effect should never fail.
 *
 * Returns TRUE if the monster's timer changed.
 */
bool mon_dec_timed(struct monster *mon, int ef_idx, int timer, u16b flag,
				   bool id)
{
	assert(ef_idx >= 0 && ef_idx < MON_TMD_MAX);
	assert(timer > 0);

	/* Decreasing is never resisted */
	flag |= MON_TMD_FLG_NOFAIL;

	/* New counter amount */
	timer = mon->m_timed[ef_idx] - timer;
	if (timer < 0)
		timer = 0;

	return mon_set_timed(mon, ef_idx, timer, flag, id);
}

/**
 * Clears the timed effect `ef_idx`.
 *
 * Returns TRUE if the monster's timer was changed.
 */
bool mon_clear_timed(struct monster *mon, int ef_idx, u16b flag, bool id)
{
	assert(ef_idx >= 0 && ef_idx < MON_TMD_MAX);

	if (!mon->m_timed[ef_idx]) return FALSE;

	/* Clearing never fails */
	flag |= MON_TMD_FLG_NOFAIL;

	return mon_set_timed(mon, ef_idx, 0, flag, id);
}

