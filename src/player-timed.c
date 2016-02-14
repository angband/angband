/**
 * \file player-timed.c
 * \brief Timed effects handling
 *
 * Copyright (c) 1997 Ben Harrison
 * Copyright (c) 2007 A Sidwell <andi@takkaria.org>
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
#include "mon-util.h"
#include "obj-identify.h"
#include "obj-knowledge.h"
#include "player-calcs.h"
#include "player-timed.h"
#include "player-util.h"


/**
 * The "stun" and "cut" statuses need to be handled by special functions of
 * their own, as they are more complex than the ones handled by the generic
 * code.
 */
static bool set_stun(struct player *p, int v);
static bool set_cut(struct player *p, int v);


static struct timed_effect {
	const char *description;
	const char *on_begin;
	const char *on_end;
	const char *on_increase;
	const char *on_decrease;
	u32b flag_redraw, flag_update;
	int msg;
	int fail_code;
	int fail;
} effects[] = {
	#define TMD(a, b, c, d, e, f, g, h, i, j, k) \
		{ b, c, d, e, f, g, h, i, j, k },
	#include "list-player-timed.h"
	#undef TMD
};

static const char *timed_name_list[] = {
	#define TMD(a, b, c, d, e, f, g, h, i, j, k) #a,
	#include "list-player-timed.h"
	#undef TMD
	"MAX",
    NULL
};

int timed_name_to_idx(const char *name)
{
    int i;
    for (i = 0; timed_name_list[i]; i++) {
        if (!my_stricmp(name, timed_name_list[i]))
            return i;
    }

    return -1;
}

const char *timed_idx_to_name(int type)
{
    assert(type >= 0);
    assert(type < TMD_MAX);

    return timed_name_list[type];
}

const char *timed_idx_to_desc(int type)
{
    assert(type >= 0);
    assert(type < TMD_MAX);

    return effects[type].description;
}

int timed_protect_flag(int type)
{
	return effects[type].fail;
}

/**
 * Set a timed event (except timed resists, cutting and stunning).
 */
bool player_set_timed(struct player *p, int idx, int v, bool notify)
{
	struct timed_effect *effect;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;
	if ((idx < 0) || (idx > TMD_MAX)) return false;

	/* No change */
	if (p->timed[idx] == v) return false;

	/* Hack -- call other functions */
	if (idx == TMD_STUN) return set_stun(p, v);
	else if (idx == TMD_CUT) return set_cut(p, v);

	/* Don't mention effects which already match the player state. */
	if (idx == TMD_OPP_ACID && player_is_immune(p, ELEM_ACID))
		notify = false;
	else if (idx == TMD_OPP_ELEC && player_is_immune(p, ELEM_ELEC))
		notify = false;
	else if (idx == TMD_OPP_FIRE && player_is_immune(p, ELEM_FIRE))
		notify = false;
	else if (idx == TMD_OPP_COLD && player_is_immune(p, ELEM_COLD))
		notify = false;
	else if (idx == TMD_OPP_CONF && player_of_has(p, OF_PROT_CONF))
		notify = false;

	/* Find the effect */
	effect = &effects[idx];

	/* Always mention start or finish, otherwise on request */
	if (v == 0) {
		msgt(MSG_RECOVER, "%s", effect->on_end);
		notify = true;
	} else if (p->timed[idx] == 0) {
		msgt(effect->msg, "%s", effect->on_begin);
		notify = true;
	} else if (notify) {
		/* Decrementing */
		if (p->timed[idx] > v && effect->on_decrease)
			msgt(effect->msg, "%s", effect->on_decrease);

		/* Incrementing */
		else if (v > p->timed[idx] && effect->on_increase)
			msgt(effect->msg, "%s", effect->on_increase);
	}

	/* Use the value */
	p->timed[idx] = v;

	/* Sort out the sprint effect */
	if (idx == TMD_SPRINT && v == 0)
		player_inc_timed(p, TMD_SLOW, 100, true, false);

	/* Nothing to notice */
	if (!notify) return false;

	/* Disturb */
	disturb(p, 0);

	/* Update the visuals, as appropriate. */
	p->upkeep->update |= effect->flag_update;
	p->upkeep->redraw |= (PR_STATUS | effect->flag_redraw);

	/* Handle stuff */
	handle_stuff(p);

	/* Result */
	return true;
}

/**
 * Increase the timed effect `idx` by `v`.  Mention this if `notify` is true.
 * Check for resistance to the effect if `check` is true.
 */
bool player_inc_timed(struct player *p, int idx, int v, bool notify, bool check)
{
	struct timed_effect *effect;

	/* Find the effect */
	effect = &effects[idx];

	/* Check we have a valid effect */
	if ((idx < 0) || (idx > TMD_MAX)) return false;

	/* Check that @ can be affected by this effect */
	if (check && effect->fail_code) {
		/* If the effect is from a monster action, extra stuff happens */
		struct monster *mon = cave->mon_current > 0 ?
			cave_monster(cave, cave->mon_current) : NULL;

		/* Determine whether an effect can be prevented by a flag */
		if (effect->fail_code == TMD_FAIL_FLAG_OBJECT) {
			/* Effect is inhibited by an object flag */
			equip_learn_flag(p, effect->fail);
			if (mon) 
				update_smart_learn(mon, player, effect->fail, 0, -1);
			if (player_of_has(p, effect->fail)) {
				if (mon)
				msg("You resist the effect!");
				return false;
			}
		} else if (effect->fail_code == TMD_FAIL_FLAG_RESIST) {
			/* Effect is inhibited by a resist */
			equip_learn_element(p, effect->fail);
			if (p->state.el_info[effect->fail].res_level > 0)
				return false;
		} else if (effect->fail_code == TMD_FAIL_FLAG_VULN) {
			/* Effect is inhibited by a vulnerability 
			 * the asymmetry with resists is OK for now - NRM */
			if (p->state.el_info[effect->fail].res_level < 0) {
				equip_learn_element(p, effect->fail);
				return false;
			}
		}

		/* Special case */
		if (idx == TMD_POISONED && p->timed[TMD_OPP_POIS])
			return false;
	}

	/* Paralysis should be non-cumulative */
	if (idx == TMD_PARALYZED && p->timed[TMD_PARALYZED] > 0)
		return false;

	/* Set v */
	v = v + p->timed[idx];

	return player_set_timed(p, idx, v, notify);
}

/**
 * Decrease the timed effect `idx` by `v`.  Mention this if `notify` is true.
 */
bool player_dec_timed(struct player *p, int idx, int v, bool notify)
{
	/* Check we have a valid effect */
	if ((idx < 0) || (idx > TMD_MAX)) return false;

	/* Set v */
	v = p->timed[idx] - v;

	return player_set_timed(p, idx, v, notify);
}

/**
 * Clear the timed effect `idx`.  Mention this if `notify` is true.
 */
bool player_clear_timed(struct player *p, int idx, bool notify)
{
	return player_set_timed(p, idx, 0, notify);
}



/**
 * Set "player->timed[TMD_STUN]", notice observable changes
 *
 * Note the special code to only notice "range" changes.
 */
static bool set_stun(struct player *p, int v)
{
	int old_aux, new_aux;

	bool notice = false;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Old state */
	if (p->timed[TMD_STUN] > 100)
		/* Knocked out */
		old_aux = 3;
	else if (p->timed[TMD_STUN] > 50)
		/* Heavy stun */
		old_aux = 2;
	else if (p->timed[TMD_STUN] > 0)
		/* Stun */
		old_aux = 1;
	else
		/* None */
		old_aux = 0;

	/* New state */
	if (v > 100)
		/* Knocked out */
		new_aux = 3;
	else if (v > 50)
		/* Heavy stun */
		new_aux = 2;
	else if (v > 0)
		/* Stun */
		new_aux = 1;
	else
		/* None */
		new_aux = 0;

	/* Increase or decrease stun */
	if (new_aux > old_aux) {
		/* Describe the state */
		switch (new_aux)
		{
			/* Stun */
			case 1:
			{
				msgt(MSG_STUN, "You have been stunned.");
				break;
			}

			/* Heavy stun */
			case 2:
			{
				msgt(MSG_STUN, "You have been heavily stunned.");
				break;
			}

			/* Knocked out */
			case 3:
			{
				msgt(MSG_STUN, "You have been knocked out.");
				break;
			}
		}

		/* Notice */
		notice = true;
	} else if (new_aux < old_aux) {
		/* Describe the state */
		switch (new_aux)
		{
			/* None */
			case 0:
			{
				msgt(MSG_RECOVER, "You are no longer stunned.");
				disturb(player, 0);
				break;
			}
		}

		/* Notice */
		notice = true;
	}

	/* Use the value */
	p->timed[TMD_STUN] = v;

	/* No change */
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
 * Set "player->timed[TMD_CUT]", notice observable changes
 *
 * Note the special code to only notice "range" changes.
 */
static bool set_cut(struct player *p, int v)
{
	int old_aux, new_aux;

	bool notice = false;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Old state */
	if (p->timed[TMD_CUT] > 1000)
		/* Mortal wound */
		old_aux = 7;
	else if (p->timed[TMD_CUT] > 200)
		/* Deep gash */
		old_aux = 6;
	else if (p->timed[TMD_CUT] > 100)
		/* Severe cut */
		old_aux = 5;
	else if (p->timed[TMD_CUT] > 50)
		/* Nasty cut */
		old_aux = 4;
	else if (p->timed[TMD_CUT] > 25)
		/* Bad cut */
		old_aux = 3;
	else if (p->timed[TMD_CUT] > 10)
		/* Light cut */
		old_aux = 2;
	else if (p->timed[TMD_CUT] > 0)
		/* Graze */
		old_aux = 1;
	else
		/* None */
		old_aux = 0;

	/* New state */
	if (v > 1000)
		/* Mortal wound */
		new_aux = 7;
	else if (v > 200)
		/* Deep gash */
		new_aux = 6;
	else if (v > 100)
		/* Severe cut */
		new_aux = 5;
	else if (v > 50)
		/* Nasty cut */
		new_aux = 4;
	else if (v > 25)
		/* Bad cut */
		new_aux = 3;
	else if (v > 10)
		/* Light cut */
		new_aux = 2;
	else if (v > 0)
		/* Graze */
		new_aux = 1;
	else
		/* None */
		new_aux = 0;

	/* Increase or decrease cut */
	if (new_aux > old_aux) {
		/* Describe the state */
		switch (new_aux)
		{
			/* Graze */
			case 1:
			{
				msgt(MSG_CUT, "You have been given a graze.");
				break;
			}

			/* Light cut */
			case 2:
			{
				msgt(MSG_CUT, "You have been given a light cut.");
				break;
			}

			/* Bad cut */
			case 3:
			{
				msgt(MSG_CUT, "You have been given a bad cut.");
				break;
			}

			/* Nasty cut */
			case 4:
			{
				msgt(MSG_CUT, "You have been given a nasty cut.");
				break;
			}

			/* Severe cut */
			case 5:
			{
				msgt(MSG_CUT, "You have been given a severe cut.");
				break;
			}

			/* Deep gash */
			case 6:
			{
				msgt(MSG_CUT, "You have been given a deep gash.");
				break;
			}

			/* Mortal wound */
			case 7:
			{
				msgt(MSG_CUT, "You have been given a mortal wound.");
				break;
			}
		}

		/* Notice */
		notice = true;
	} else if (new_aux < old_aux) {
		/* Describe the state */
		switch (new_aux)
		{
			/* None */
			case 0:
			{
				msgt(MSG_RECOVER, "You are no longer bleeding.");
				disturb(player, 0);
				break;
			}
		}

		/* Notice */
		notice = true;
	}

	/* Use the value */
	p->timed[TMD_CUT] = v;

	/* No change */
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


