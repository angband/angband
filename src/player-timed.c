/**
 * \file player-timed.h
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
#include "player-timed.h"
#include "player-util.h"


/**
 * The "stun" and "cut" statuses need to be handled by special functions of
 * their own, as they are more complex than the ones handled by the generic
 * code.
 */
static bool set_stun(struct player *p, int v);
static bool set_cut(struct player *p, int v);


static timed_effect effects[] =
{
	#define TMD(a, b, c, d, e, f, g, h, i, j) { b, c, d, e, f, g, h, i, j },
	#include "list-player-timed.h"
	#undef TMD
};

static const char *timed_name_list[] = {
	#define TMD(a, b, c, d, e, f, g, h, i, j) #a,
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

int timed_protect_flag(int type)
{
	return effects[type].fail;
}

/*
 * Set a timed event (except timed resists, cutting and stunning).
 */
bool player_set_timed(struct player *p, int idx, int v, bool notify)
{
	timed_effect *effect;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;
	if ((idx < 0) || (idx > TMD_MAX)) return FALSE;

	/* No change */
	if (p->timed[idx] == v) return FALSE;

	/* Hack -- call other functions */
	if (idx == TMD_STUN) return set_stun(p, v);
	else if (idx == TMD_CUT) return set_cut(p, v);

	/* Don't mention effects which already match the player state. */
	if (idx == TMD_OPP_ACID && player_is_immune(p, ELEM_ACID))
		notify = FALSE;
	else if (idx == TMD_OPP_ELEC && player_is_immune(p, ELEM_ELEC))
		notify = FALSE;
	else if (idx == TMD_OPP_FIRE && player_is_immune(p, ELEM_FIRE))
		notify = FALSE;
	else if (idx == TMD_OPP_COLD && player_is_immune(p, ELEM_COLD))
		notify = FALSE;
	else if (idx == TMD_OPP_CONF && player_of_has(p, OF_PROT_CONF))
		notify = FALSE;

	/* Find the effect */
	effect = &effects[idx];

	/* Turning off, always mention */
	if (v == 0)
	{
		msgt(MSG_RECOVER, "%s", effect->on_end);
		notify = TRUE;
	}

	/* Turning on, always mention */
	else if (p->timed[idx] == 0)
	{
		msgt(effect->msg, "%s", effect->on_begin);
		notify = TRUE;
	}

	else if (notify)
	{
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
		player_inc_timed(p, TMD_SLOW, 100, TRUE, FALSE);

	/* Nothing to notice */
	if (!notify) return FALSE;

	/* Disturb */
	disturb(p, 0);

	/* Update the visuals, as appropriate. */
	p->upkeep->update |= effect->flag_update;
	p->upkeep->redraw |= (PR_STATUS | effect->flag_redraw);

	/* Handle stuff */
	handle_stuff(p->upkeep);

	/* Result */
	return TRUE;
}

/**
 * Increase the timed effect `idx` by `v`.  Mention this if `notify` is TRUE.
 * Check for resistance to the effect if `check` is TRUE.
 */
bool player_inc_timed(struct player *p, int idx, int v, bool notify, bool check)
{
	timed_effect *effect;

	/* Find the effect */
	effect = &effects[idx];

	/* Check we have a valid effect */
	if ((idx < 0) || (idx > TMD_MAX)) return FALSE;

	/* Check that @ can be affected by this effect */
	if (check && effects->fail_code) {
		/* If the effect is from a monster action, extra stuff happens */
		struct monster *mon = cave->mon_current > 0 ?
			cave_monster(cave, cave->mon_current) : NULL;

		/* This is all a bit gross - NRM */
		if (effects->fail_code == 1) {
			/* Code 1 is an object flag */
			equip_notice_flag(p, effect->fail);
			if (mon) 
				update_smart_learn(mon, player, effect->fail, 0, -1);
			if (player_of_has(p, effect->fail)) {
				if (mon)
				msg("You resist the effect!");
				return FALSE;
			}
		} else if (effects->fail_code == 2) {
			/* Code 2 is a resist */
			equip_notice_element(p, effect->fail);
			if (p->state.el_info[effect->fail].res_level > 0)
				return FALSE;
		} else if (effects->fail_code == 2) {
			/* Code 3 is a vulnerability */
			equip_notice_element(p, effect->fail);
			if (p->state.el_info[effect->fail].res_level < 0)
				return FALSE;
		}

		/* Special case */
		if (idx == TMD_POISONED && p->timed[TMD_OPP_POIS])
			return FALSE;
	}

	/* Paralysis should be non-cumulative */
	if (idx == TMD_PARALYZED && p->timed[TMD_PARALYZED] > 0)
		return FALSE;

	/* Set v */
	v = v + p->timed[idx];

	return player_set_timed(p, idx, v, notify);
}

/**
 * Decrease the timed effect `idx` by `v`.  Mention this if `notify` is TRUE.
 */
bool player_dec_timed(struct player *p, int idx, int v, bool notify)
{
	/* Check we have a valid effect */
	if ((idx < 0) || (idx > TMD_MAX)) return FALSE;

	/* Set v */
	v = p->timed[idx] - v;

	return player_set_timed(p, idx, v, notify);
}

/**
 * Clear the timed effect `idx`.  Mention this if `notify` is TRUE.
 */
bool player_clear_timed(struct player *p, int idx, bool notify)
{
	return player_set_timed(p, idx, 0, notify);
}



/*
 * Set "player->timed[TMD_STUN]", notice observable changes
 *
 * Note the special code to only notice "range" changes.
 */
static bool set_stun(struct player *p, int v)
{
	int old_aux, new_aux;

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Knocked out */
	if (p->timed[TMD_STUN] > 100)
	{
		old_aux = 3;
	}

	/* Heavy stun */
	else if (p->timed[TMD_STUN] > 50)
	{
		old_aux = 2;
	}

	/* Stun */
	else if (p->timed[TMD_STUN] > 0)
	{
		old_aux = 1;
	}

	/* None */
	else
	{
		old_aux = 0;
	}

	/* Knocked out */
	if (v > 100)
	{
		new_aux = 3;
	}

	/* Heavy stun */
	else if (v > 50)
	{
		new_aux = 2;
	}

	/* Stun */
	else if (v > 0)
	{
		new_aux = 1;
	}

	/* None */
	else
	{
		new_aux = 0;
	}

	/* Increase cut */
	if (new_aux > old_aux)
	{
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
		notice = TRUE;
	}

	/* Decrease cut */
	else if (new_aux < old_aux)
	{
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
		notice = TRUE;
	}

	/* Use the value */
	p->timed[TMD_STUN] = v;

	/* No change */
	if (!notice) return (FALSE);

	/* Disturb */
	disturb(player, 0);

	/* Recalculate bonuses */
	p->upkeep->update |= (PU_BONUS);

	/* Redraw the "stun" */
	p->upkeep->redraw |= (PR_STATUS);

	/* Handle stuff */
	handle_stuff(player->upkeep);

	/* Result */
	return (TRUE);
}


/*
 * Set "player->timed[TMD_CUT]", notice observable changes
 *
 * Note the special code to only notice "range" changes.
 */
static bool set_cut(struct player *p, int v)
{
	int old_aux, new_aux;

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Mortal wound */
	if (p->timed[TMD_CUT] > 1000)
	{
		old_aux = 7;
	}

	/* Deep gash */
	else if (p->timed[TMD_CUT] > 200)
	{
		old_aux = 6;
	}

	/* Severe cut */
	else if (p->timed[TMD_CUT] > 100)
	{
		old_aux = 5;
	}

	/* Nasty cut */
	else if (p->timed[TMD_CUT] > 50)
	{
		old_aux = 4;
	}

	/* Bad cut */
	else if (p->timed[TMD_CUT] > 25)
	{
		old_aux = 3;
	}

	/* Light cut */
	else if (p->timed[TMD_CUT] > 10)
	{
		old_aux = 2;
	}

	/* Graze */
	else if (p->timed[TMD_CUT] > 0)
	{
		old_aux = 1;
	}

	/* None */
	else
	{
		old_aux = 0;
	}

	/* Mortal wound */
	if (v > 1000)
	{
		new_aux = 7;
	}

	/* Deep gash */
	else if (v > 200)
	{
		new_aux = 6;
	}

	/* Severe cut */
	else if (v > 100)
	{
		new_aux = 5;
	}

	/* Nasty cut */
	else if (v > 50)
	{
		new_aux = 4;
	}

	/* Bad cut */
	else if (v > 25)
	{
		new_aux = 3;
	}

	/* Light cut */
	else if (v > 10)
	{
		new_aux = 2;
	}

	/* Graze */
	else if (v > 0)
	{
		new_aux = 1;
	}

	/* None */
	else
	{
		new_aux = 0;
	}

	/* Increase cut */
	if (new_aux > old_aux)
	{
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
		notice = TRUE;
	}

	/* Decrease cut */
	else if (new_aux < old_aux)
	{
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
		notice = TRUE;
	}

	/* Use the value */
	p->timed[TMD_CUT] = v;

	/* No change */
	if (!notice) return (FALSE);

	/* Disturb */
	disturb(player, 0);

	/* Recalculate bonuses */
	p->upkeep->update |= (PU_BONUS);

	/* Redraw the "cut" */
	p->upkeep->redraw |= (PR_STATUS);

	/* Handle stuff */
	handle_stuff(player->upkeep);

	/* Result */
	return (TRUE);
}


/*
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

	bool notice = FALSE;

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

	/* Food increase */
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
		notice = TRUE;
	}

	/* Food decrease */
	else if (new_aux < old_aux) {
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
		notice = TRUE;
	}

	/* Use the value */
	p->food = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	disturb(player, 0);

	/* Recalculate bonuses */
	p->upkeep->update |= (PU_BONUS);

	/* Redraw hunger */
	p->upkeep->redraw |= (PR_STATUS);

	/* Handle stuff */
	handle_stuff(player->upkeep);

	/* Result */
	return (TRUE);
}


