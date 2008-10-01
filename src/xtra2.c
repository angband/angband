/*
 * File: xtra2.c
 * Purpose: Targetting, sorting, panel update, timed effects handling, monster
 * death
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
#include "cmds.h"
#include "object/tvalsval.h"

/*** Timed effects ***/

/*
 * This code replace a lot of virtually identical functions and (ostensibly)
 * is a lot cleaner.  Note that the various "oppose" functions and the "stun"
 * and "cut" statuses need to be handled by special functions of their own,
 * as they are more complex than the ones handled by the generic code.  -AS-
 */
static bool set_oppose_acid(int v);
static bool set_oppose_elec(int v);
static bool set_oppose_fire(int v);
static bool set_oppose_cold(int v);
static bool set_oppose_conf(int v);
static bool set_stun(int v);
static bool set_cut(int v);


typedef struct
{
  const char *on_begin, *on_end;
  u32b flag_redraw, flag_update;
  int msg;
} timed_effect;

static timed_effect effects[] =
{
	{ "You feel yourself moving faster!", "You feel yourself slow down.", 0, PU_BONUS, MSG_SPEED },
	{ "You feel yourself moving slower!", "You feel yourself speed up.", 0, PU_BONUS, MSG_SLOW },
	{ "You are blind.", "You can see again.", (PR_MAP), (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_MONSTERS), MSG_BLIND },
	{ "You are paralyzed!", "You can move again.", 0, 0, MSG_PARALYZED },
	{ "You are confused!", "You feel less confused now.", 0, 0, MSG_CONFUSED },
	{ "You are terrified!", "You feel bolder now.", 0, 0, MSG_AFRAID },
	{ "You feel drugged!", "You can see clearly again.", (PR_MAP), 0, MSG_DRUGGED },
	{ "You are poisoned!", "You are no longer poisoned.", 0, 0, MSG_POISONED },
	{ "", "", 0, 0, 0 },  /* TMD_CUT -- handled seperately */
	{ "", "", 0, 0, 0 },  /* TMD_STUN -- handled seperately */
	{ "You feel safe from evil!", "You no longer feel safe from evil.", 0, 0, MSG_PROT_EVIL },
	{ "You feel invulnerable!", "You feel vulnerable once more.", 0, PU_BONUS, MSG_INVULN },
	{ "You feel like a hero!", "The heroism wears off.", 0, PU_BONUS, MSG_HERO },
	{ "You feel like a killing machine!", "You feel less Berserk.", 0, PU_BONUS, MSG_BERSERK },
	{ "A mystic shield forms around your body!", "Your mystic shield crumbles away.", 0, PU_BONUS, MSG_SHIELD },
	{ "You feel righteous!", "The prayer has expired.", 0, PU_BONUS, MSG_BLESSED },
	{ "Your eyes feel very sensitive!", "Your eyes feel less sensitive.", 0, (PU_BONUS | PU_MONSTERS), MSG_SEE_INVIS },
	{ "Your eyes begin to tingle!", "Your eyes stop tingling.", 0, (PU_BONUS | PU_MONSTERS), MSG_INFRARED },
	{ "", "", 0, 0, 0 },  /* acid -- handled seperately */
	{ "", "", 0, 0, 0 },  /* elec -- handled seperately */
	{ "", "", 0, 0, 0 },  /* fire -- handled seperately */
	{ "", "", 0, 0, 0 },  /* cold -- handled seperately */
	{ "", "", 0, 0, 0 },  /* conf -- handled seperately */
	{ "You feel resistant to poison!", "You feel less resistant to poison.", 0, 0, MSG_RES_POIS },
	{ "You feel your memories fade.", "Your memories come flooding back.", 0, 0, MSG_GENERIC },
	{ "Your mind expands.", "Your horizons are once more limited.", 0, PU_BONUS, MSG_GENERIC },
	{ "Your skin turns to stone.", "A fleshy shade returns to your skin.", 0, PU_BONUS, MSG_GENERIC },
	{ "You feel the need to run away, and fast!", "The urge to run dissipates.", 0, PU_BONUS, MSG_AFRAID },
	{ "You start sprinting.", "You suddenly stop sprinting.", 0, PU_BONUS, MSG_SPEED },
};

/*
 * Set a timed event (except timed resists, cutting and stunning).
 */
bool set_timed(int idx, int v)
{
	bool notice = FALSE;
	timed_effect *effect;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;
	if ((idx < 0) || (idx > TMD_MAX)) return FALSE;

	/* Hack -- call other functions */
	if (idx == TMD_STUN) return set_stun(v);
	else if (idx == TMD_CUT) return set_cut(v);
	else if (idx == TMD_OPP_ACID) return set_oppose_acid(v);
	else if (idx == TMD_OPP_ELEC) return set_oppose_elec(v);
	else if (idx == TMD_OPP_FIRE) return set_oppose_fire(v);
	else if (idx == TMD_OPP_COLD) return set_oppose_cold(v);
	else if (idx == TMD_OPP_CONF) return set_oppose_conf(v);

	/* Find the effect */
	effect = &effects[idx];

	/* Open */
	if (v)
	{
		if (!p_ptr->timed[idx])
		{
			message(effect->msg, 0, effect->on_begin);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->timed[idx])
		{
			message(MSG_RECOVER, 0, effect->on_end);
			notice = TRUE;

			if (idx == TMD_SPRINT)
				inc_timed(TMD_SLOW, 100);
		}
	}


	/* Use the value */
	p_ptr->timed[idx] = v;

	/* Nothing to notice */
	if (!notice) return FALSE;

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Update the visuals, as appropriate. */
	p_ptr->update |= effect->flag_update;
	p_ptr->redraw |= (PR_STATUS | effect->flag_redraw);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return TRUE;
}

bool inc_timed(int idx, int v)
{
	/* Check we have a valid effect */
	if ((idx < 0) || (idx > TMD_MAX)) return FALSE;

	/* Set v */
	v = v + p_ptr->timed[idx];

	return set_timed(idx, v);
}

bool dec_timed(int idx, int v)
{
	/* Check we have a valid effect */
	if ((idx < 0) || (idx > TMD_MAX)) return FALSE;

	/* Set v */
	v = p_ptr->timed[idx] - v;

	return set_timed(idx, v);
}

bool clear_timed(int idx)
{
	return set_timed(idx, 0);
}


/*
 * Set "p_ptr->timed[TMD_OPP_ACID]", notice observable changes
 */
static bool set_oppose_acid(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->timed[TMD_OPP_ACID] && !p_ptr->immune_acid)
		{
			message(MSG_RES_ACID, 0, "You feel resistant to acid!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->timed[TMD_OPP_ACID] && !p_ptr->immune_acid)
		{
			message(MSG_RECOVER, 0, "You feel less resistant to acid.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->timed[TMD_OPP_ACID] = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Redraw */
	p_ptr->redraw |= PR_STATUS;

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->timed[TMD_OPP_ELEC]", notice observable changes
 */
static bool set_oppose_elec(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->timed[TMD_OPP_ELEC] && !p_ptr->immune_elec)
		{
			message(MSG_RES_ELEC, 0, "You feel resistant to electricity!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->timed[TMD_OPP_ELEC] && !p_ptr->immune_elec)
		{
			message(MSG_RECOVER, 0, "You feel less resistant to electricity.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->timed[TMD_OPP_ELEC] = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Redraw */
	p_ptr->redraw |= PR_STATUS;

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->timed[TMD_OPP_FIRE]", notice observable changes
 */
static bool set_oppose_fire(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->timed[TMD_OPP_FIRE] && !p_ptr->immune_fire)
		{
			message(MSG_RES_FIRE, 0, "You feel resistant to fire!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->timed[TMD_OPP_FIRE] && !p_ptr->immune_fire)
		{
			message(MSG_RECOVER, 0, "You feel less resistant to fire.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->timed[TMD_OPP_FIRE] = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Redraw */
	p_ptr->redraw |= PR_STATUS;

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->timed[TMD_OPP_COLD]", notice observable changes
 */
static bool set_oppose_cold(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->timed[TMD_OPP_COLD] && !p_ptr->immune_cold)
		{
			message(MSG_RES_COLD, 0, "You feel resistant to cold!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->timed[TMD_OPP_COLD] && !p_ptr->immune_cold)
		{
			message(MSG_RECOVER, 0, "You feel less resistant to cold.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->timed[TMD_OPP_COLD] = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Redraw */
	p_ptr->redraw |= PR_STATUS;

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->timed[TMD_OPP_CONF]", notice observable changes
 */
static bool set_oppose_conf(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->timed[TMD_OPP_CONF] && !p_ptr->resist_confu)
		{
			message(MSG_RES_ELEC, 0, "You feel remarkably clear-headed!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->timed[TMD_OPP_CONF] && !p_ptr->resist_confu)
		{
			message(MSG_RECOVER, 0, "You feel less clear-headed.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->timed[TMD_OPP_CONF] = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Redraw */
	p_ptr->update |= PU_BONUS;
	p_ptr->redraw |= PR_STATUS;

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}



/*
 * Set "p_ptr->timed[TMD_STUN]", notice observable changes
 *
 * Note the special code to only notice "range" changes.
 */
static bool set_stun(int v)
{
	int old_aux, new_aux;

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Knocked out */
	if (p_ptr->timed[TMD_STUN] > 100)
	{
		old_aux = 3;
	}

	/* Heavy stun */
	else if (p_ptr->timed[TMD_STUN] > 50)
	{
		old_aux = 2;
	}

	/* Stun */
	else if (p_ptr->timed[TMD_STUN] > 0)
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
				message(MSG_STUN, 0, "You have been stunned.");
				break;
			}

			/* Heavy stun */
			case 2:
			{
				message(MSG_STUN, 0, "You have been heavily stunned.");
				break;
			}

			/* Knocked out */
			case 3:
			{
				message(MSG_STUN, 0, "You have been knocked out.");
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
				message(MSG_RECOVER, 0, "You are no longer stunned.");
				if (disturb_state) disturb(0, 0);
				break;
			}
		}

		/* Notice */
		notice = TRUE;
	}

	/* Use the value */
	p_ptr->timed[TMD_STUN] = v;

	/* No change */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw the "stun" */
	p_ptr->redraw |= (PR_STATUS);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->timed[TMD_CUT]", notice observable changes
 *
 * Note the special code to only notice "range" changes.
 */
static bool set_cut(int v)
{
	int old_aux, new_aux;

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Mortal wound */
	if (p_ptr->timed[TMD_CUT] > 1000)
	{
		old_aux = 7;
	}

	/* Deep gash */
	else if (p_ptr->timed[TMD_CUT] > 200)
	{
		old_aux = 6;
	}

	/* Severe cut */
	else if (p_ptr->timed[TMD_CUT] > 100)
	{
		old_aux = 5;
	}

	/* Nasty cut */
	else if (p_ptr->timed[TMD_CUT] > 50)
	{
		old_aux = 4;
	}

	/* Bad cut */
	else if (p_ptr->timed[TMD_CUT] > 25)
	{
		old_aux = 3;
	}

	/* Light cut */
	else if (p_ptr->timed[TMD_CUT] > 10)
	{
		old_aux = 2;
	}

	/* Graze */
	else if (p_ptr->timed[TMD_CUT] > 0)
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
				message(MSG_CUT, 0, "You have been given a graze.");
				break;
			}

			/* Light cut */
			case 2:
			{
				message(MSG_CUT, 0, "You have been given a light cut.");
				break;
			}

			/* Bad cut */
			case 3:
			{
				message(MSG_CUT, 0, "You have been given a bad cut.");
				break;
			}

			/* Nasty cut */
			case 4:
			{
				message(MSG_CUT, 0, "You have been given a nasty cut.");
				break;
			}

			/* Severe cut */
			case 5:
			{
				message(MSG_CUT, 0, "You have been given a severe cut.");
				break;
			}

			/* Deep gash */
			case 6:
			{
				message(MSG_CUT, 0, "You have been given a deep gash.");
				break;
			}

			/* Mortal wound */
			case 7:
			{
				message(MSG_CUT, 0, "You have been given a mortal wound.");
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
				message(MSG_RECOVER, 0, "You are no longer bleeding.");
				if (disturb_state) disturb(0, 0);
				break;
			}
		}

		/* Notice */
		notice = TRUE;
	}

	/* Use the value */
	p_ptr->timed[TMD_CUT] = v;

	/* No change */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw the "cut" */
	p_ptr->redraw |= (PR_STATUS);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->food", notice observable changes
 *
 * The "p_ptr->food" variable can get as large as 20000, allowing the
 * addition of the most "filling" item, Elvish Waybread, which adds
 * 7500 food units, without overflowing the 32767 maximum limit.
 *
 * Perhaps we should disturb the player with various messages,
 * especially messages about hunger status changes.  XXX XXX XXX
 *
 * Digestion of food is handled in "dungeon.c", in which, normally,
 * the player digests about 20 food units per 100 game turns, more
 * when "fast", more when "regenerating", less with "slow digestion",
 * but when the player is "gorged", he digests 100 food units per 10
 * game turns, or a full 1000 food units per 100 game turns.
 *
 * Note that the player's speed is reduced by 10 units while gorged,
 * so if the player eats a single food ration (5000 food units) when
 * full (15000 food units), he will be gorged for (5000/100)*10 = 500
 * game turns, or 500/(100/5) = 25 player turns (if nothing else is
 * affecting the player speed).
 */
bool set_food(int v)
{
	int old_aux, new_aux;

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = MIN(v, PY_FOOD_UPPER);
	v = MAX(v, 0);

	/* Fainting / Starving */
	if (p_ptr->food < PY_FOOD_FAINT)
	{
		old_aux = 0;
	}

	/* Weak */
	else if (p_ptr->food < PY_FOOD_WEAK)
	{
		old_aux = 1;
	}

	/* Hungry */
	else if (p_ptr->food < PY_FOOD_ALERT)
	{
		old_aux = 2;
	}

	/* Normal */
	else if (p_ptr->food < PY_FOOD_FULL)
	{
		old_aux = 3;
	}

	/* Full */
	else if (p_ptr->food < PY_FOOD_MAX)
	{
		old_aux = 4;
	}

	/* Gorged */
	else
	{
		old_aux = 5;
	}

	/* Fainting / Starving */
	if (v < PY_FOOD_FAINT)
	{
		new_aux = 0;
	}

	/* Weak */
	else if (v < PY_FOOD_WEAK)
	{
		new_aux = 1;
	}

	/* Hungry */
	else if (v < PY_FOOD_ALERT)
	{
		new_aux = 2;
	}

	/* Normal */
	else if (v < PY_FOOD_FULL)
	{
		new_aux = 3;
	}

	/* Full */
	else if (v < PY_FOOD_MAX)
	{
		new_aux = 4;
	}

	/* Gorged */
	else
	{
		new_aux = 5;
	}

	/* Food increase */
	if (new_aux > old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* Weak */
			case 1:
			{
				msg_print("You are still weak.");
				break;
			}

			/* Hungry */
			case 2:
			{
				msg_print("You are still hungry.");
				break;
			}

			/* Normal */
			case 3:
			{
				msg_print("You are no longer hungry.");
				break;
			}

			/* Full */
			case 4:
			{
				msg_print("You are full!");
				break;
			}

			/* Bloated */
			case 5:
			{
				msg_print("You have gorged yourself!");
				break;
			}
		}

		/* Change */
		notice = TRUE;
	}

	/* Food decrease */
	else if (new_aux < old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* Fainting / Starving */
			case 0:
			{
				sound(MSG_NOTICE);
				msg_print("You are getting faint from hunger!");
				break;
			}

			/* Weak */
			case 1:
			{
				sound(MSG_NOTICE);
				msg_print("You are getting weak from hunger!");
				break;
			}

			/* Hungry */
			case 2:
			{
				sound(MSG_HUNGRY);
				msg_print("You are getting hungry.");
				break;
			}

			/* Normal */
			case 3:
			{
				sound(MSG_NOTICE);
				msg_print("You are no longer full.");
				break;
			}

			/* Full */
			case 4:
			{
				sound(MSG_NOTICE);
				msg_print("You are no longer gorged.");
				break;
			}
		}

		/* Change */
		notice = TRUE;
	}

	/* Use the value */
	p_ptr->food = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw hunger */
	p_ptr->redraw |= (PR_STATUS);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}





/*
 * Advance experience levels and print experience
 */
void check_experience(void)
{
	/* Hack -- lower limit */
	if (p_ptr->exp < 0) p_ptr->exp = 0;

	/* Hack -- lower limit */
	if (p_ptr->max_exp < 0) p_ptr->max_exp = 0;

	/* Hack -- upper limit */
	if (p_ptr->exp > PY_MAX_EXP) p_ptr->exp = PY_MAX_EXP;

	/* Hack -- upper limit */
	if (p_ptr->max_exp > PY_MAX_EXP) p_ptr->max_exp = PY_MAX_EXP;


	/* Hack -- maintain "max" experience */
	if (p_ptr->exp > p_ptr->max_exp) p_ptr->max_exp = p_ptr->exp;

	/* Redraw experience */
	p_ptr->redraw |= (PR_EXP);

	/* Handle stuff */
	handle_stuff();


	/* Lose levels while possible */
	while ((p_ptr->lev > 1) &&
	       (p_ptr->exp < (player_exp[p_ptr->lev-2] *
	                      p_ptr->expfact / 100L)))
	{
		/* Lose a level */
		p_ptr->lev--;

		/* Update some stuff */
		p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

		/* Redraw some stuff */
		p_ptr->redraw |= (PR_LEV | PR_TITLE | PR_EXP);

		/* Handle stuff */
		handle_stuff();
	}


	/* Gain levels while possible */
	while ((p_ptr->lev < PY_MAX_LEVEL) &&
	       (p_ptr->exp >= (player_exp[p_ptr->lev-1] *
	                       p_ptr->expfact / 100L)))
	{
		char buf[80];

		/* Gain a level */
		p_ptr->lev++;

		/* Save the highest level */
		if (p_ptr->lev > p_ptr->max_lev)
		{
			p_ptr->max_lev = p_ptr->lev;

			/* Log level updates (TODO: perhaps only every other level or every 5) */
			strnfmt(buf, sizeof(buf), "Reached level %d", p_ptr->lev);
			history_add(buf, HISTORY_GAIN_LEVEL, 0);
		}

		/* Message */
		message_format(MSG_LEVEL, p_ptr->lev, "Welcome to level %d.", p_ptr->lev);

		/* Add to social class */
		p_ptr->sc += randint1(2);
		if (p_ptr->sc > 150) p_ptr->sc = 150;

		/* Update some stuff */
		p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

		/* Redraw some stuff */
		p_ptr->redraw |= (PR_LEV | PR_TITLE | PR_EXP);

		/* Handle stuff */
		handle_stuff();
	}

	/* Gain max levels while possible */
	while ((p_ptr->max_lev < PY_MAX_LEVEL) &&
	       (p_ptr->max_exp >= (player_exp[p_ptr->max_lev-1] *
	                           p_ptr->expfact / 100L)))
	{
		/* Gain max level */
		p_ptr->max_lev++;

		/* Update some stuff */
		p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

		/* Redraw some stuff */
		p_ptr->redraw |= (PR_LEV | PR_TITLE | PR_EXP);

		/* Handle stuff */
		handle_stuff();
	}
}


/*
 * Gain experience
 */
void gain_exp(s32b amount)
{
	/* Gain some experience */
	p_ptr->exp += amount;

	/* Slowly recover from experience drainage */
	if (p_ptr->exp < p_ptr->max_exp)
	{
		/* Gain max experience (10%) */
		p_ptr->max_exp += amount / 10;
	}

	/* Check Experience */
	check_experience();
}


/*
 * Lose experience
 */
void lose_exp(s32b amount)
{
	/* Never drop below zero experience */
	if (amount > p_ptr->exp) amount = p_ptr->exp;

	/* Lose some experience */
	p_ptr->exp -= amount;

	/* Check Experience */
	check_experience();
}




/*
 * Hack -- Return the "automatic coin type" of a monster race
 * Used to allocate proper treasure when "Creeping coins" die
 *
 * Note the use of actual "monster names".  XXX XXX XXX
 */
static int get_coin_type(const monster_race *r_ptr)
{
	cptr name = (r_name + r_ptr->name);

	/* Analyze "coin" monsters */
	if (r_ptr->d_char == '$')
	{
		/* Look for textual clues */
		if (strstr(name, " copper ")) return (SV_COPPER);
		if (strstr(name, " silver ")) return (SV_SILVER);
		if (strstr(name, " gold ")) return (SV_GOLD);
		if (strstr(name, " mithril ")) return (SV_MITHRIL);
		if (strstr(name, " adamantite ")) return (SV_ADAMANTITE);

		/* Look for textual clues */
		if (strstr(name, "Copper ")) return (SV_COPPER);
		if (strstr(name, "Silver ")) return (SV_SILVER);
		if (strstr(name, "Gold ")) return (SV_GOLD);
		if (strstr(name, "Mithril ")) return (SV_MITHRIL);
		if (strstr(name, "Adamantite ")) return (SV_ADAMANTITE);
	}

	/* Assume nothing */
	return (SV_GOLD_ANY);
}


/*
 * Create magical stairs after finishing a quest monster.
 */
static void build_quest_stairs(int y, int x)
{
	int ny, nx;


	/* Stagger around */
	while (!cave_valid_bold(y, x))
	{
		int d = 1;

		/* Pick a location */
		scatter(&ny, &nx, y, x, d, 0);

		/* Stagger */
		y = ny; x = nx;
	}

	/* Destroy any objects */
	delete_object(y, x);

	/* Explain the staircase */
	msg_print("A magical staircase appears...");

	/* Create stairs down */
	cave_set_feat(y, x, FEAT_MORE);

	/* Update the visuals */
	p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

	/* Fully update the flow */
	p_ptr->update |= (PU_FORGET_FLOW | PU_UPDATE_FLOW);
}


/*
 * Handle the "death" of a monster.
 *
 * Disperse treasures centered at the monster location based on the
 * various flags contained in the monster flags fields.
 *
 * Check for "Quest" completion when a quest monster is killed.
 *
 * Note that only the player can induce "monster_death()" on Uniques.
 * Thus (for now) all Quest monsters should be Uniques.
 *
 * Note that monsters can now carry objects, and when a monster dies,
 * it drops all of its objects, which may disappear in crowded rooms.
 */
void monster_death(int m_idx)
{
	int i, j, y, x, level;

	int dump_item = 0;
	int dump_gold = 0;

	int number = 0;
	int total = 0;

	s16b this_o_idx, next_o_idx = 0;

	monster_type *m_ptr = &mon_list[m_idx];

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	bool visible = (m_ptr->ml || (r_ptr->flags[0] & (RF0_UNIQUE)));

	bool great = (r_ptr->flags[0] & (RF0_DROP_GREAT)) ? TRUE : FALSE;
	bool good = ((r_ptr->flags[0] & (RF0_DROP_GOOD)) ? TRUE : FALSE) || great;

	bool gold_ok = (!(r_ptr->flags[0] & (RF0_ONLY_ITEM)));
	bool item_ok = (!(r_ptr->flags[0] & (RF0_ONLY_GOLD)));

	int force_coin = get_coin_type(r_ptr);

	object_type *i_ptr;
	object_type object_type_body;


	/* Get the location */
	y = m_ptr->fy;
	x = m_ptr->fx;


	/* Drop objects being carried */
	for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		/* Get the object */
		o_ptr = &o_list[this_o_idx];

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Paranoia */
		o_ptr->held_m_idx = 0;

		/* Get local object */
		i_ptr = &object_type_body;

		/* Copy the object */
		object_copy(i_ptr, o_ptr);

		/* Delete the object */
		delete_object_idx(this_o_idx);

		/* Drop it */
		drop_near(i_ptr, -1, y, x);
	}

	/* Forget objects */
	m_ptr->hold_o_idx = 0;


	/* Mega-Hack -- drop "winner" treasures */
	if (r_ptr->flags[0] & (RF0_DROP_CHOSEN))
	{
		/* Get local object */
		i_ptr = &object_type_body;

		/* Mega-Hack -- Prepare to make "Grond" */
		object_prep(i_ptr, lookup_kind(TV_HAFTED, SV_GROND));

		/* Mega-Hack -- Mark this item as "Grond" */
		i_ptr->name1 = ART_GROND;

		/* Mega-Hack -- Actually create "Grond" */
		apply_magic(i_ptr, -1, TRUE, TRUE, TRUE);
		i_ptr->origin = ORIGIN_DROP;
		i_ptr->origin_depth = p_ptr->depth;
		i_ptr->origin_xtra = m_ptr->r_idx;

		/* Drop it in the dungeon */
		drop_near(i_ptr, -1, y, x);


		/* Get local object */
		i_ptr = &object_type_body;

		/* Mega-Hack -- Prepare to make "Morgoth" */
		object_prep(i_ptr, lookup_kind(TV_CROWN, SV_MORGOTH));
		i_ptr->origin = ORIGIN_DROP;
		i_ptr->origin_depth = p_ptr->depth;
		i_ptr->origin_xtra = m_ptr->r_idx;

		/* Mega-Hack -- Mark this item as "Morgoth" */
		i_ptr->name1 = ART_MORGOTH;

		/* Mega-Hack -- Actually create "Morgoth" */
		apply_magic(i_ptr, -1, TRUE, TRUE, TRUE);

		/* Drop it in the dungeon */
		drop_near(i_ptr, -1, y, x);
	}


	/* Determine how much we can drop */
	if ((r_ptr->flags[0] & RF0_DROP_20) && (randint0(100) < 20)) number++;
	if ((r_ptr->flags[0] & RF0_DROP_40) && (randint0(100) < 40)) number++;
	if ((r_ptr->flags[0] & RF0_DROP_60) && (randint0(100) < 60)) number++;

	if (r_ptr->flags[0] & RF0_DROP_4) number += rand_range(2, 6);
	if (r_ptr->flags[0] & RF0_DROP_3) number += rand_range(2, 4);
	if (r_ptr->flags[0] & RF0_DROP_2) number += rand_range(1, 3);
	if (r_ptr->flags[0] & RF0_DROP_1) number++;

	/* Average monster level and current depth */
	level = (p_ptr->depth + r_ptr->level) / 2;

	/* Drop some objects */
	for (j = 0; j < number; j++)
	{
		/* Get local object */
		i_ptr = &object_type_body;

		/* Wipe the object */
		object_wipe(i_ptr);

		/* Make Gold */
		if (gold_ok && (!item_ok || (randint0(100) < 50)))
		{
			/* Make some gold */
			make_gold(i_ptr, level, force_coin);
			dump_gold++;
		}

		/* Make Object */
		else
		{
			/* Make an object */
			if (!make_object(i_ptr, level, good, great)) continue;
			dump_item++;
		}

		/* Set origin */
		i_ptr->origin = visible ? ORIGIN_DROP : ORIGIN_DROP_UNKNOWN;
		i_ptr->origin_depth = p_ptr->depth;
		i_ptr->origin_xtra = m_ptr->r_idx;

		/* Drop it in the dungeon */
		drop_near(i_ptr, -1, y, x);
	}

	/* Take note of any dropped treasure */
	if (visible && (dump_item || dump_gold))
	{
		/* Take notes on treasure */
		lore_treasure(m_idx, dump_item, dump_gold);
	}

	/* Update monster list window */
	p_ptr->redraw |= PR_MONLIST;

	/* Only process "Quest Monsters" */
	if (!(r_ptr->flags[0] & (RF0_QUESTOR))) return;

	/* Hack -- Mark quests as complete */
	for (i = 0; i < MAX_Q_IDX; i++)
	{
		/* Hack -- note completed quests */
		if (q_list[i].level == r_ptr->level) q_list[i].level = 0;

		/* Count incomplete quests */
		if (q_list[i].level) total++;
	}

	/* Build magical stairs */
	build_quest_stairs(y, x);

	/* Nothing left, game over... */
	if (total == 0)
	{
		/* Total winner */
		p_ptr->total_winner = TRUE;

		/* Redraw the "title" */
		p_ptr->redraw |= (PR_TITLE);

		/* Congratulations */
		msg_print("*** CONGRATULATIONS ***");
		msg_print("You have won the game!");
		msg_print("You may retire (commit suicide) when you are ready.");
	}
}




/*
 * Decrease a monster's hit points, handle monster death.
 *
 * We return TRUE if the monster has been killed (and deleted).
 *
 * We announce monster death (using an optional "death message"
 * if given, and a otherwise a generic killed/destroyed message).
 *
 * Only "physical attacks" can induce the "You have slain" message.
 * Missile and Spell attacks will induce the "dies" message, or
 * various "specialized" messages.  Note that "You have destroyed"
 * and "is destroyed" are synonyms for "You have slain" and "dies".
 *
 * Invisible monsters induce a special "You have killed it." message.
 *
 * Hack -- we "delay" fear messages by passing around a "fear" flag.
 *
 * Consider decreasing monster experience over time, say, by using
 * "(m_exp * m_lev * (m_lev)) / (p_lev * (m_lev + n_killed))" instead
 * of simply "(m_exp * m_lev) / (p_lev)", to make the first monster
 * worth more than subsequent monsters.  This would also need to
 * induce changes in the monster recall code.  XXX XXX XXX
 */
bool mon_take_hit(int m_idx, int dam, bool *fear, cptr note)
{
	monster_type *m_ptr = &mon_list[m_idx];

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	monster_lore *l_ptr = &l_list[m_ptr->r_idx];

	s32b div, new_exp, new_exp_frac;


	/* Redraw (later) if needed */
	if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);


	/* Wake it up */
	m_ptr->csleep = 0;

	/* Hurt it */
	m_ptr->hp -= dam;

	/* It is dead now */
	if (m_ptr->hp < 0)
	{
		char m_name[80];
		char buf[80];

		/* Assume normal death sound */
		int soundfx = MSG_KILL;

		/* Play a special sound if the monster was unique */
		if (r_ptr->flags[0] & RF0_UNIQUE) 
		{
			/* Mega-Hack -- Morgoth -- see monster_death() */
			if (r_ptr->flags[0] & RF0_DROP_CHOSEN)
				soundfx = MSG_KILL_KING;
			else
				soundfx = MSG_KILL_UNIQUE;
		}

		/* Extract monster name */
		monster_desc(m_name, sizeof(m_name), m_ptr, 0);

		/* Death by Missile/Spell attack */
		if (note)
		{
			message_format(soundfx, m_ptr->r_idx, "%^s%s", m_name, note);
		}

		/* Death by physical attack -- invisible monster */
		else if (!m_ptr->ml)
		{
			message_format(soundfx, m_ptr->r_idx, "You have killed %s.", m_name);
		}

		/* Death by Physical attack -- non-living monster */
		else if ((r_ptr->flags[2] & (RF2_DEMON | RF2_UNDEAD)) ||
		         (r_ptr->flags[1] & (RF1_STUPID)) ||
		         (strchr("Evg", r_ptr->d_char)))
		{
			message_format(soundfx, m_ptr->r_idx, "You have destroyed %s.", m_name);
		}

		/* Death by Physical attack -- living monster */
		else
		{
			message_format(soundfx, m_ptr->r_idx, "You have slain %s.", m_name);
		}

		/* Player level */
		div = p_ptr->lev;

		/* Give some experience for the kill */
		new_exp = ((long)r_ptr->mexp * r_ptr->level) / div;

		/* Handle fractional experience */
		new_exp_frac = ((((long)r_ptr->mexp * r_ptr->level) % div)
		                * 0x10000L / div) + p_ptr->exp_frac;

		/* Keep track of experience */
		if (new_exp_frac >= 0x10000L)
		{
			new_exp++;
			p_ptr->exp_frac = (u16b)(new_exp_frac - 0x10000L);
		}
		else
		{
			p_ptr->exp_frac = (u16b)new_exp_frac;
		}

		/* When the player kills a Unique, it stays dead */
		if (r_ptr->flags[0] & (RF0_UNIQUE))
		{
			char unique_name[80];
			r_ptr->max_num = 0;

			/* This gets the correct name if we slay an invisible unique and don't have See Invisible. */
			monster_desc(unique_name, sizeof(unique_name), m_ptr, MDESC_SHOW | MDESC_IND2);

			/* Log the slaying of a unique */
			strnfmt(buf, sizeof(buf), "Killed %s", unique_name);
			history_add(buf, HISTORY_SLAY_UNIQUE, 0);
		}

		/* Gain experience */
		gain_exp(new_exp);

		/* Generate treasure */
		monster_death(m_idx);

		/* Recall even invisible uniques or winners */
		if (m_ptr->ml || (r_ptr->flags[0] & (RF0_UNIQUE)))
		{
			/* Count kills this life */
			if (l_ptr->pkills < MAX_SHORT) l_ptr->pkills++;

			/* Count kills in all lives */
			if (l_ptr->tkills < MAX_SHORT) l_ptr->tkills++;

			/* Hack -- Auto-recall */
			monster_race_track(m_ptr->r_idx);
		}

		/* Delete the monster */
		delete_monster_idx(m_idx);

		/* Not afraid */
		(*fear) = FALSE;

		/* Monster is dead */
		return (TRUE);
	}


	/* Mega-Hack -- Pain cancels fear */
	if (m_ptr->monfear && (dam > 0))
	{
		int tmp = randint1(dam);

		/* Cure a little fear */
		if (tmp < m_ptr->monfear)
		{
			/* Reduce fear */
			m_ptr->monfear -= tmp;
		}

		/* Cure all the fear */
		else
		{
			/* Cure fear */
			m_ptr->monfear = 0;

			/* No more fear */
			(*fear) = FALSE;
		}
	}

	/* Sometimes a monster gets scared by damage */
	if (!m_ptr->monfear && !(r_ptr->flags[2] & (RF2_NO_FEAR)) && (dam > 0))
	{
		int percentage;

		/* Percentage of fully healthy */
		percentage = (100L * m_ptr->hp) / m_ptr->maxhp;

		/*
		 * Run (sometimes) if at 10% or less of max hit points,
		 * or (usually) when hit for half its current hit points
		 */
		if ((randint1(10) >= percentage) ||
		    ((dam >= m_ptr->hp) && (randint0(100) < 80)))
		{
			/* Hack -- note fear */
			(*fear) = TRUE;

			/* Hack -- Add some timed fear */
			m_ptr->monfear = (randint1(10) +
			                  (((dam >= m_ptr->hp) && (percentage > 7)) ?
			                   20 : ((11 - percentage) * 5)));
		}
	}


	/* Not dead yet */
	return (FALSE);
}


/*
 * Modify the current panel to the given coordinates, adjusting only to
 * ensure the coordinates are legal, and return TRUE if anything done.
 *
 * The town should never be scrolled around.
 *
 * Note that monsters are no longer affected in any way by panel changes.
 *
 * As a total hack, whenever the current panel changes, we assume that
 * the "overhead view" window should be updated.
 */
bool modify_panel(term *t, int wy, int wx)
{
	int dungeon_hgt = (p_ptr->depth == 0) ? TOWN_HGT : DUNGEON_HGT;
	int dungeon_wid = (p_ptr->depth == 0) ? TOWN_WID : DUNGEON_WID;

	int screen_hgt = (t == Term) ? (t->hgt - ROW_MAP - 1) : t->hgt;
	int screen_wid = (t == Term) ? (t->wid - COL_MAP - 1) : t->wid;

	/* Bigtile panels only have half the width */
	if (use_bigtile) screen_wid = screen_wid / 2;

	/* Verify wy, adjust if needed */
	if (wy > dungeon_hgt - screen_hgt) wy = dungeon_hgt - screen_hgt;
	if (wy < 0) wy = 0;

	/* Verify wx, adjust if needed */
	if (wx > dungeon_wid - screen_wid) wx = dungeon_wid - screen_wid;
	if (wx < 0) wx = 0;

	/* React to changes */
	if ((t->offset_y != wy) || (t->offset_x != wx))
	{
		/* Save wy, wx */
		t->offset_y = wy;
		t->offset_x = wx;

		/* Redraw map */
		p_ptr->redraw |= (PR_MAP);

		/* Changed */
		return (TRUE);
	}

	/* No change */
	return (FALSE);
}


/*
 * Perform the minimum "whole panel" adjustment to ensure that the given
 * location is contained inside the current panel, and return TRUE if any
 * such adjustment was performed.
 */
bool adjust_panel(int y, int x)
{
	bool changed = FALSE;

	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		int wx, wy;
		int screen_hgt, screen_wid;

		term *t = angband_term[j];

		/* No window */
		if (!t) continue;

		/* No relevant flags */
		if ((j > 0) && !(op_ptr->window_flag[j] & PW_MAP)) continue;

		wy = t->offset_y;
		wx = t->offset_x;

		screen_hgt = (j == 0) ? (Term->hgt - ROW_MAP - 1) : t->hgt;
		screen_wid = (j == 0) ? (Term->wid - COL_MAP - 1) : t->wid;

		/* Bigtile panels only have half the width */
		if (use_bigtile) screen_wid = screen_wid / 2;

		/* Adjust as needed */
		while (y >= wy + screen_hgt) wy += screen_hgt / 2;
		while (y < wy) wy -= screen_hgt / 2;

		/* Adjust as needed */
		while (x >= wx + screen_wid) wx += screen_wid / 2;
		while (x < wx) wx -= screen_wid / 2;

		/* Use "modify_panel" */
		if (modify_panel(t, wy, wx)) changed = TRUE;
	}

	return (changed);
}


/*
 * Change the current panel to the panel lying in the given direction.
 *
 * Return TRUE if the panel was changed.
 */
bool change_panel(int dir)
{
	bool changed = FALSE;
	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		int screen_hgt, screen_wid;
		int wx, wy;

		term *t = angband_term[j];

		/* No window */
		if (!t) continue;

		/* No relevant flags */
		if ((j > 0) && !(op_ptr->window_flag[j] & PW_MAP)) continue;

		screen_hgt = (j == 0) ? (Term->hgt - ROW_MAP - 1) : t->hgt;
		screen_wid = (j == 0) ? (Term->wid - COL_MAP - 1) : t->wid;

		/* Bigtile panels only have half the width */
		if (use_bigtile) screen_wid = screen_wid / 2;

		/* Shift by half a panel */
		wy = t->offset_y + ddy[dir] * screen_hgt / 2;
		wx = t->offset_x + ddx[dir] * screen_wid / 2;

		/* Use "modify_panel" */
		if (modify_panel(t, wy, wx)) changed = TRUE;
	}

	return (changed);
}


/*
 * Verify the current panel (relative to the player location).
 *
 * By default, when the player gets "too close" to the edge of the current
 * panel, the map scrolls one panel in that direction so that the player
 * is no longer so close to the edge.
 *
 * The "center_player" option allows the current panel to always be centered
 * around the player, which is very expensive, and also has some interesting
 * gameplay ramifications.
 */
void verify_panel(void)
{
	int wy, wx;
	int screen_hgt, screen_wid;

	int panel_wid, panel_hgt;

	int py = p_ptr->py;
	int px = p_ptr->px;

	int j;

	/* Scan windows */
	for (j = 0; j < ANGBAND_TERM_MAX; j++)
	{
		term *t = angband_term[j];

		/* No window */
		if (!t) continue;

		/* No relevant flags */
		if ((j > 0) && !(op_ptr->window_flag[j] & (PW_MAP))) continue;

		wy = t->offset_y;
		wx = t->offset_x;

		screen_hgt = (j == 0) ? (Term->hgt - ROW_MAP - 1) : t->hgt;
		screen_wid = (j == 0) ? (Term->wid - COL_MAP - 1) : t->wid;

		/* Bigtile panels only have half the width */
		if (use_bigtile) screen_wid = screen_wid / 2;

		panel_wid = screen_wid / 2;
		panel_hgt = screen_hgt / 2;


		/* Scroll screen vertically when off-center */
		if (center_player && !p_ptr->running && (py != wy + panel_hgt))
			wy = py - panel_hgt;

		/* Scroll screen vertically when 3 grids from top/bottom edge */
		else if ((py < wy + 3) || (py >= wy + screen_hgt - 3))
			wy = py - panel_hgt;


		/* Scroll screen horizontally when off-center */
		if (center_player && !p_ptr->running && (px != wx + panel_wid))
			wx = px - panel_wid;

		/* Scroll screen horizontally when 3 grids from left/right edge */
		else if ((px < wx + 3) || (px >= wx + screen_wid - 3))
			wx = px - panel_wid;


		/* Scroll if needed */
		modify_panel(t, wy, wx);
	}
}


/*
 * Given a "source" and "target" location, extract a "direction",
 * which will move one step from the "source" towards the "target".
 *
 * Note that we use "diagonal" motion whenever possible.
 *
 * We return "5" if no motion is needed.
 */
int motion_dir(int y1, int x1, int y2, int x2)
{
	/* No movement required */
	if ((y1 == y2) && (x1 == x2)) return (5);

	/* South or North */
	if (x1 == x2) return ((y1 < y2) ? 2 : 8);

	/* East or West */
	if (y1 == y2) return ((x1 < x2) ? 6 : 4);

	/* South-east or South-west */
	if (y1 < y2) return ((x1 < x2) ? 3 : 1);

	/* North-east or North-west */
	if (y1 > y2) return ((x1 < x2) ? 9 : 7);

	/* Paranoia */
	return (5);
}


/*
 * Extract a direction (or zero) from a character
 */
int target_dir(char ch)
{
	int d = 0;

	int mode;

	cptr act;

	cptr s;


	/* Already a direction? */
	if (isdigit((unsigned char)ch))
	{
		d = D2I(ch);
	}
	else if (isarrow(ch))
	{
		switch (ch)
		{
			case ARROW_DOWN:	d = 2; break;
			case ARROW_LEFT:	d = 4; break;
			case ARROW_RIGHT:	d = 6; break;
			case ARROW_UP:		d = 8; break;
		}
	}
	else
	{
		/* Roguelike */
		if (rogue_like_commands)
		{
			mode = KEYMAP_MODE_ROGUE;
		}

		/* Original */
		else
		{
			mode = KEYMAP_MODE_ORIG;
		}

		/* Extract the action (if any) */
		act = keymap_act[mode][(byte)(ch)];

		/* Analyze */
		if (act)
		{
			/* Convert to a direction */
			for (s = act; *s; ++s)
			{
				/* Use any digits in keymap */
				if (isdigit((unsigned char)*s)) d = D2I(*s);
			}
		}
	}

	/* Paranoia */
	if (d == 5) d = 0;

	/* Return direction */
	return (d);
}


int dir_transitions[10][10] = 
{
	/* 0-> */ { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },
	/* 1-> */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* 2-> */ { 0, 0, 2, 0, 1, 0, 3, 0, 5, 0 }, 
	/* 3-> */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* 4-> */ { 0, 0, 1, 0, 4, 0, 5, 0, 7, 0 }, 
	/* 5-> */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* 6-> */ { 0, 0, 3, 0, 5, 0, 6, 0, 9, 0 }, 
	/* 7-> */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* 8-> */ { 0, 0, 5, 0, 7, 0, 9, 0, 8, 0 }, 
	/* 9-> */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};


/*
 * Get an "aiming direction" (1,2,3,4,6,7,8,9 or 5) from the user.
 *
 * Return TRUE if a direction was chosen, otherwise return FALSE.
 *
 * The direction "5" is special, and means "use current target".
 *
 * This function tracks and uses the "global direction", and uses
 * that as the "desired direction", if it is set.
 *
 * Note that "Force Target", if set, will pre-empt user interaction,
 * if there is a usable target already set.
 *
 * Currently this function applies confusion directly.
 */
bool get_aim_dir(int *dp)
{
	int dir;

	ui_event_data ke;

	cptr p;

	if (repeat_pull(dp))
	{
		/* Verify */
		if (!(*dp == 5 && !target_okay()))
		{
			return (TRUE);
		}
		else
		{
			/* Invalid repeat - reset it */
			repeat_clear();
		}
	}

	/* Initialize */
	(*dp) = 0;

	/* Global direction */
	dir = p_ptr->command_dir;

	/* Hack -- auto-target if requested */
	if (use_old_target && target_okay()) dir = 5;

	/* Ask until satisfied */
	while (!dir)
	{
		/* Choose a prompt */
		if (!target_okay())
		{
			p = "Direction ('*' or <click> to choose a target, Escape to cancel)? ";
		}
		else
		{
			p = "Direction ('5' for target, '*' or <click> to re-target, Escape to cancel)? ";
		}

		/* Get a command (or Cancel) */
		if (!get_com_ex(p, &ke)) break;

		/* Analyze */
		switch (ke.key)
		{
			/* Mouse aiming */
			case '\xff':
			{
				if (target_set_interactive(TARGET_KILL, KEY_GRID_X(ke), KEY_GRID_Y(ke)))
					dir = 5;

				break;
			}

			/* Set new target, use target if legal */
			case '*':
			{
				if (target_set_interactive(TARGET_KILL, -1, -1)) dir = 5;
				break;
			}

			/* Use current target, if set and legal */
			case 't':
			case '5':
			case '0':
			case '.':
			{
				if (target_okay()) dir = 5;
				break;
			}

			/* Possible direction */
			default:
			{
				int keypresses_handled = 0;
				
				while (ke.key != 0)
				{
					int this_dir;
					
					/* XXX Ideally show and move the cursor here to indicate 
					   the currently "Pending" direction. XXX */
					this_dir = target_dir(ke.key);
					
					if (this_dir)
					{
						dir = dir_transitions[dir][this_dir];
					}
					else
					{
						break;
					}
					
					if (lazymove_delay == 0 || ++keypresses_handled > 1)
						break;
				
					/* See if there's a second keypress within the defined
					   period of time. */
					inkey_scan = lazymove_delay; 
					ke = inkey_ex();
				}
			}
		}

		/* Error */
		if (!dir) bell("Illegal aim direction!");
	}

	/* No direction */
	if (!dir) return (FALSE);

	/* Save the direction */
	p_ptr->command_dir = dir;

	/* Check for confusion */
	if (p_ptr->timed[TMD_CONFUSED])
	{
		/* Random direction */
		dir = ddd[randint0(8)];
	}

	/* Notice confusion */
	if (p_ptr->command_dir != dir)
	{
		/* Warn the user */
		msg_print("You are confused.");
	}

	/* Save direction */
	(*dp) = dir;

	repeat_push(dir);

	/* A "valid" direction was entered */
	return (TRUE);
}


/*
 * Request a "movement" direction (1,2,3,4,6,7,8,9) from the user.
 *
 * Return TRUE if a direction was chosen, otherwise return FALSE.
 *
 * This function should be used for all "repeatable" commands, such as
 * run, walk, open, close, bash, disarm, spike, tunnel, etc, as well
 * as all commands which must reference a grid adjacent to the player,
 * and which may not reference the grid under the player.
 *
 * Directions "5" and "0" are illegal and will not be accepted.
 *
 * This function tracks and uses the "global direction", and uses
 * that as the "desired direction", if it is set.
 */
bool get_rep_dir(int *dp)
{
	int dir;

	ui_event_data ke;

	if (repeat_pull(dp))
	{
		return (TRUE);
	}

	/* Initialize */
	(*dp) = 0;

	/* Global direction */
	dir = p_ptr->command_dir;

	/* Get a direction */
	while (!dir)
	{
		/* Paranoia XXX XXX XXX */
		message_flush();

		/* Get first keypress - the first test is to avoid displaying the
		   prompt for direction if there's already a keypress queued up
		   and waiting - this just avoids a flickering prompt if there is
		   a "lazy" movement delay. */
		inkey_scan = SCAN_INSTANT;
		ke = inkey_ex();
		inkey_scan = SCAN_OFF;

		if (ke.key != '\xff' && target_dir(ke.key) == 0)
		{
			prt("Direction or <click> (Escape to cancel)? ", 0, 0);
			ke = inkey_ex();
		}

		/* Check mouse coordinates */
		if (ke.key == '\xff')
		{
			/*if (ke.button) */
			{
				int y = KEY_GRID_Y(ke);
				int x = KEY_GRID_X(ke);

				/* Calculate approximate angle */
				int angle = get_angle_to_target(p_ptr->py, p_ptr->px, y, x, 0);

				/* Convert angle to direction */
				if (angle < 15) dir = 6;
				else if (angle < 33) dir = 9;
				else if (angle < 59) dir = 8;
				else if (angle < 78) dir = 7;
				else if (angle < 104) dir = 4;
				else if (angle < 123) dir = 1;
				else if (angle < 149) dir = 2;
				else if (angle < 168) dir = 3;
				else dir = 6;
			}
		}

		/* Get other keypresses until a direction is chosen. */
		else 
		{
			int keypresses_handled = 0;

			while (ke.key != 0)
			{
				int this_dir;

				if (ke.key == ESCAPE) 
				{
					/* Clear the prompt */
					prt("", 0, 0);

					return (FALSE);
				}

				/* XXX Ideally show and move the cursor here to indicate 
				   the currently "Pending" direction. XXX */
				this_dir = target_dir(ke.key);

				if (this_dir)
				{
					dir = dir_transitions[dir][this_dir];
				}

				if (lazymove_delay == 0 || ++keypresses_handled > 1)
					break;

				inkey_scan = lazymove_delay; 
				ke = inkey_ex();
			}

			/* 5 is equivalent to "escape" */
			if (dir == 5)
			{
				/* Clear the prompt */
				prt("", 0, 0);

				return (FALSE);
			}
		}

		/* Oops */
		if (!dir) bell("Illegal repeatable direction!");
	}

	/* Clear the prompt */
	prt("", 0, 0);

	/* Aborted */
	if (!dir) return (FALSE);

	/* Save desired direction */
	p_ptr->command_dir = dir;

	/* Save direction */
	(*dp) = dir;

	repeat_push(dir);

	/* Success */
	return (TRUE);
}


/*
 * Apply confusion, if needed, to a direction
 *
 * Display a message and return TRUE if direction changes.
 */
bool confuse_dir(int *dp)
{
	int dir;

	/* Default */
	dir = (*dp);

	/* Apply "confusion" */
	if (p_ptr->timed[TMD_CONFUSED])
	{
		/* Apply confusion XXX XXX XXX */
		if ((dir == 5) || (randint0(100) < 75))
		{
			/* Random direction */
			dir = ddd[randint0(8)];
		}
	}

	/* Notice confusion */
	if ((*dp) != dir)
	{
		/* Warn the user */
		msg_print("You are confused.");

		/* Save direction */
		(*dp) = dir;

		/* Confused */
		return (TRUE);
	}

	/* Not confused */
	return (FALSE);
}



