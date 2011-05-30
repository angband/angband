/*
 * File: effects.c
 * Purpose: Big switch statement for every effect in the game
 *
 * Copyright (c) 2007 Andrew Sidwell
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
#include "monster/mon-spell.h"
#include "trap.h"
#include "spells.h"

/*
 * Entries for spell/activation descriptions
 */
typedef struct
{
	u16b index;          /* Effect index */
	bool aim;            /* Whether the effect requires aiming */
	u16b power;	     /* Power rating for obj-power.c */
	const char *desc;    /* Effect description */
} info_entry;

/*
 * Useful things about effects.
 */
static const info_entry effects[] =
{
	#define EFFECT(x, y, r, z)    { EF_##x, y, r, z },
	#include "list-effects.h"
	#undef EFFECT
};


/*
 * Utility functions
 */
bool effect_aim(effect_type effect)
{
	if (effect < 1 || effect > EF_MAX)
		return FALSE;

	return effects[effect].aim;
}

int effect_power(effect_type effect)
{
	if (effect < 1 || effect > EF_MAX)
		return FALSE;

	return effects[effect].power;
}

const char *effect_desc(effect_type effect)
{
	if (effect < 1 || effect > EF_MAX)
		return FALSE;

	return effects[effect].desc;
}

bool effect_obvious(effect_type effect)
{
	if (effect == EF_IDENTIFY)
		return TRUE;

	return FALSE;
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
	int py = p_ptr->py;
	int px = p_ptr->px;
	int plev = p_ptr->lev;

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




/*
 * Do an effect, given an object.
 * Boost is the extent to which skill surpasses difficulty, used as % boost. It
 * ranges from 0 to 138.
 */
bool effect_do(effect_type effect, bool *ident, bool aware, int dir, int beam,
	int boost)
{
	int py = p_ptr->py;
	int px = p_ptr->px;
	int dam, chance, dur;

	if (effect < 1 || effect > EF_MAX)
	{
		msg("Bad effect passed to do_effect().  Please report this bug.");
		return FALSE;
	}

	switch (effect)
	{
		case EF_POISON:
		{
			player_inc_timed(p_ptr, TMD_POISONED, damroll(2, 7) + 10, TRUE, TRUE);
			*ident = TRUE;
			return TRUE;
		}

		case EF_BLIND:
		{
			player_inc_timed(p_ptr, TMD_BLIND, damroll(4, 25) + 75, TRUE, TRUE);
			*ident = TRUE;
			return TRUE;
		}

		case EF_SCARE:
		{
			player_inc_timed(p_ptr, TMD_AFRAID, randint0(10) + 10, TRUE, TRUE);
			*ident = TRUE;
			return TRUE;
		}

		case EF_CONFUSE:
		{
			player_inc_timed(p_ptr, TMD_CONFUSED, damroll(4, 5) + 10, TRUE, TRUE);
			*ident = TRUE;
			return TRUE;
		}

		case EF_HALLUC:
		{
			player_inc_timed(p_ptr, TMD_IMAGE, randint0(250) + 250, TRUE, TRUE);
			*ident = TRUE;
			return TRUE;
		}

		case EF_PARALYZE:
		{
			player_inc_timed(p_ptr, TMD_PARALYZED, randint0(5) + 5, TRUE, TRUE);
			*ident = TRUE;
			return TRUE;
		}

		case EF_SLOW:
		{
			if (player_inc_timed(p_ptr, TMD_SLOW, randint1(25) + 15, TRUE, TRUE))
				*ident = TRUE;
			return TRUE;
		}

		case EF_CURE_POISON:
		{
			if (player_clear_timed(p_ptr, TMD_POISONED, TRUE)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURE_BLINDNESS:
		{
			if (player_clear_timed(p_ptr, TMD_BLIND, TRUE)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURE_PARANOIA:
		{
			if (player_clear_timed(p_ptr, TMD_AFRAID, TRUE)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURE_CONFUSION:
		{
			if (player_clear_timed(p_ptr, TMD_CONFUSED, TRUE)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURE_MIND:
		{
			if (player_clear_timed(p_ptr, TMD_CONFUSED, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_AFRAID, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_IMAGE, TRUE)) *ident = TRUE;
			if (!of_has(p_ptr->state.flags, OF_RES_CONFU) &&
				player_inc_timed(p_ptr, TMD_OPP_CONF, damroll(4, 10), TRUE, TRUE))
			    	*ident = TRUE;
			return TRUE;
		}

		case EF_CURE_BODY:
		{
			if (player_clear_timed(p_ptr, TMD_STUN, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_CUT, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_POISONED, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_BLIND, TRUE)) *ident = TRUE;
			return TRUE;
		}


		case EF_CURE_LIGHT:
		{
			if (hp_player(20)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_BLIND, TRUE)) *ident = TRUE;
			if (player_dec_timed(p_ptr, TMD_CUT, 20, TRUE)) *ident = TRUE;
			if (player_dec_timed(p_ptr, TMD_CONFUSED, 20, TRUE)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURE_SERIOUS:
		{
			if (hp_player(40)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_CUT, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_BLIND, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_CONFUSED, TRUE)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURE_CRITICAL:
		{
			if (hp_player(60)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_BLIND, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_CONFUSED, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_POISONED, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_STUN, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_CUT, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_AMNESIA, TRUE)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURE_FULL:
		{
			int amt = (p_ptr->mhp * 35) / 100;
			if (amt < 300) amt = 300;

			if (hp_player(amt)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_BLIND, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_CONFUSED, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_POISONED, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_STUN, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_CUT, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_AMNESIA, TRUE)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURE_FULL2:
		{
			if (hp_player(1200)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_BLIND, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_CONFUSED, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_POISONED, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_STUN, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_CUT, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_AMNESIA, TRUE)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURE_TEMP:
		{
			if (player_clear_timed(p_ptr, TMD_BLIND, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_POISONED, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_CONFUSED, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_STUN, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_CUT, TRUE)) *ident = TRUE;
			return TRUE;
		}

		case EF_HEAL1:
		{
			if (hp_player(500)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_CUT, TRUE)) *ident = TRUE;
			return TRUE;
		}

		case EF_HEAL2:
		{
			if (hp_player(1000)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_CUT, TRUE)) *ident = TRUE;
			return TRUE;
		}

		case EF_HEAL3:
		{
			if (hp_player(500)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_STUN, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_CUT, TRUE)) *ident = TRUE;
			return TRUE;
		}

		case EF_GAIN_EXP:
		{
			if (p_ptr->exp < PY_MAX_EXP)
			{
				msg("You feel more experienced.");
				player_exp_gain(p_ptr, 100000L);
				*ident = TRUE;
			}
			return TRUE;
		}

		case EF_LOSE_EXP:
		{
			if (!check_state(p_ptr, OF_HOLD_LIFE, p_ptr->state.flags) && (p_ptr->exp > 0))
			{
				msg("You feel your memories fade.");
				player_exp_lose(p_ptr, p_ptr->exp / 4, FALSE);
				*ident = TRUE;
			}
			*ident = TRUE;
			wieldeds_notice_flag(p_ptr, OF_HOLD_LIFE);
			return TRUE;
		}

		case EF_RESTORE_EXP:
		{
			if (restore_level()) *ident = TRUE;
			return TRUE;
		}

		case EF_RESTORE_MANA:
		{
			if (p_ptr->csp < p_ptr->msp)
			{
				p_ptr->csp = p_ptr->msp;
				p_ptr->csp_frac = 0;
				msg("Your feel your head clear.");
				p_ptr->redraw |= (PR_MANA);
				*ident = TRUE;
			}
			return TRUE;
		}

		case EF_GAIN_STR:
		case EF_GAIN_INT:
		case EF_GAIN_WIS:
		case EF_GAIN_DEX:
		case EF_GAIN_CON:
		case EF_GAIN_CHR:
		{
			int stat = effect - EF_GAIN_STR;
			if (do_inc_stat(stat)) *ident = TRUE;
			return TRUE;
		}

		case EF_GAIN_ALL:
		{
			if (do_inc_stat(A_STR)) *ident = TRUE;
			if (do_inc_stat(A_INT)) *ident = TRUE;
			if (do_inc_stat(A_WIS)) *ident = TRUE;
			if (do_inc_stat(A_DEX)) *ident = TRUE;
			if (do_inc_stat(A_CON)) *ident = TRUE;
			if (do_inc_stat(A_CHR)) *ident = TRUE;
			return TRUE;
		}

		case EF_BRAWN:
		{
			/* Pick a random stat to decrease other than strength */
			int stat = randint0(A_MAX-1) + 1;

			if (do_dec_stat(stat, TRUE))
			{
				do_inc_stat(A_STR);
				*ident = TRUE;
			}

			return TRUE;
		}

		case EF_INTELLECT:
		{
			/* Pick a random stat to decrease other than intelligence */
			int stat = randint0(A_MAX-1);
			if (stat >= A_INT) stat++;

			if (do_dec_stat(stat, TRUE))
			{
				do_inc_stat(A_INT);
				*ident = TRUE;
			}

			return TRUE;
		}

		case EF_CONTEMPLATION:
		{
			/* Pick a random stat to decrease other than wisdom */
			int stat = randint0(A_MAX-1);
			if (stat >= A_WIS) stat++;

			if (do_dec_stat(stat, TRUE))
			{
				do_inc_stat(A_WIS);
				*ident = TRUE;
			}

			return TRUE;
		}

		case EF_TOUGHNESS:
		{
			/* Pick a random stat to decrease other than constitution */
			int stat = randint0(A_MAX-1);
			if (stat >= A_CON) stat++;

			if (do_dec_stat(stat, TRUE))
			{
				do_inc_stat(A_CON);
				*ident = TRUE;
			}

			return TRUE;
		}

		case EF_NIMBLENESS:
		{
			/* Pick a random stat to decrease other than dexterity */
			int stat = randint0(A_MAX-1);
			if (stat >= A_DEX) stat++;

			if (do_dec_stat(stat, TRUE))
			{
				do_inc_stat(A_DEX);
				*ident = TRUE;
			}

			return TRUE;
		}

		case EF_PLEASING:
		{
			/* Pick a random stat to decrease other than charisma */
			int stat = randint0(A_MAX-1);

			if (do_dec_stat(stat, TRUE))
			{
				do_inc_stat(A_CHR);
				*ident = TRUE;
			}

			return TRUE;
		}

		case EF_LOSE_STR:
		case EF_LOSE_INT:
		case EF_LOSE_WIS:
		case EF_LOSE_DEX:
		case EF_LOSE_CON:
		case EF_LOSE_CHR:
		{
			int stat = effect - EF_LOSE_STR;

			take_hit(p_ptr, damroll(5, 5), "stat drain");
			(void)do_dec_stat(stat, FALSE);
			*ident = TRUE;

			return TRUE;
		}

		case EF_LOSE_CON2:
		{
			take_hit(p_ptr, damroll(10, 10), "poisonous food");
			(void)do_dec_stat(A_CON, FALSE);
			*ident = TRUE;

			return TRUE;
		}

		case EF_RESTORE_STR:
		case EF_RESTORE_INT:
		case EF_RESTORE_WIS:
		case EF_RESTORE_DEX:
		case EF_RESTORE_CON:
		case EF_RESTORE_CHR:
		{
			int stat = effect - EF_RESTORE_STR;
			if (do_res_stat(stat)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURE_NONORLYBIG:
		{
			msg("You feel life flow through your body!");
			restore_level();
			(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
			(void)player_clear_timed(p_ptr, TMD_BLIND, TRUE);
			(void)player_clear_timed(p_ptr, TMD_CONFUSED, TRUE);
			(void)player_clear_timed(p_ptr, TMD_IMAGE, TRUE);
			(void)player_clear_timed(p_ptr, TMD_STUN, TRUE);
			(void)player_clear_timed(p_ptr, TMD_CUT, TRUE);
			(void)player_clear_timed(p_ptr, TMD_AMNESIA, TRUE);

			if (do_res_stat(A_STR)) *ident = TRUE;
			if (do_res_stat(A_INT)) *ident = TRUE;
			if (do_res_stat(A_WIS)) *ident = TRUE;
			if (do_res_stat(A_DEX)) *ident = TRUE;
			if (do_res_stat(A_CON)) *ident = TRUE;
			if (do_res_stat(A_CHR)) *ident = TRUE;

			/* Recalculate max. hitpoints */
			update_stuff(p_ptr);

			hp_player(5000);

			*ident = TRUE;
			return TRUE;
		}

		case EF_RESTORE_ALL:
		{
			/* Life, above, also gives these effects */
			if (do_res_stat(A_STR)) *ident = TRUE;
			if (do_res_stat(A_INT)) *ident = TRUE;
			if (do_res_stat(A_WIS)) *ident = TRUE;
			if (do_res_stat(A_DEX)) *ident = TRUE;
			if (do_res_stat(A_CON)) *ident = TRUE;
			if (do_res_stat(A_CHR)) *ident = TRUE;
			return TRUE;
		}

		case EF_RESTORE_ST_LEV:
		{
			if (restore_level()) *ident = TRUE;
			if (do_res_stat(A_STR)) *ident = TRUE;
			if (do_res_stat(A_INT)) *ident = TRUE;
			if (do_res_stat(A_WIS)) *ident = TRUE;
			if (do_res_stat(A_DEX)) *ident = TRUE;
			if (do_res_stat(A_CON)) *ident = TRUE;
			if (do_res_stat(A_CHR)) *ident = TRUE;
			return TRUE;
		}

		case EF_TMD_INFRA:
		{
			if (player_inc_timed(p_ptr, TMD_SINFRA, 100 + damroll(4, 25), TRUE, TRUE))
				*ident = TRUE;
			return TRUE;
		}

		case EF_TMD_SINVIS:
		{
			if (player_clear_timed(p_ptr, TMD_BLIND, TRUE)) *ident = TRUE;
			if (player_inc_timed(p_ptr, TMD_SINVIS, 12 + damroll(2, 6), TRUE, TRUE))
				*ident = TRUE;
			return TRUE;
		}

		case EF_TMD_ESP:
		{
			if (player_clear_timed(p_ptr, TMD_BLIND, TRUE)) *ident = TRUE;
			if (player_inc_timed(p_ptr, TMD_TELEPATHY, 12 + damroll(6, 6), TRUE, TRUE))
				*ident = TRUE;
			return TRUE;
		}


		case EF_ENLIGHTENMENT:
		{
			msg("An image of your surroundings forms in your mind...");
			wiz_light();
			*ident = TRUE;
			return TRUE;
		}


		case EF_ENLIGHTENMENT2:
		{
			msg("You begin to feel more enlightened...");
			message_flush();
			wiz_light();
			(void)do_inc_stat(A_INT);
			(void)do_inc_stat(A_WIS);
			(void)detect_traps(TRUE);
			(void)detect_doorstairs(TRUE);
			(void)detect_treasure(TRUE);
			identify_pack();
			*ident = TRUE;
			return TRUE;
		}

		case EF_HERO:
		{
			dur = randint1(25) + 25;
			if (hp_player(10)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_AFRAID, TRUE)) *ident = TRUE;
			if (player_inc_timed(p_ptr, TMD_BOLD, dur, TRUE, TRUE)) *ident = TRUE;
			if (player_inc_timed(p_ptr, TMD_HERO, dur, TRUE, TRUE)) *ident = TRUE;
			return TRUE;
		}

		case EF_SHERO:
		{
			dur = randint1(25) + 25;
			if (hp_player(30)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_AFRAID, TRUE)) *ident = TRUE;
			if (player_inc_timed(p_ptr, TMD_BOLD, dur, TRUE, TRUE)) *ident = TRUE;
			if (player_inc_timed(p_ptr, TMD_SHERO, dur, TRUE, TRUE)) *ident = TRUE;
			return TRUE;
		}


		case EF_RESIST_ACID:
		{
			if (player_inc_timed(p_ptr, TMD_OPP_ACID, randint1(10) + 10, TRUE, TRUE))
				*ident = TRUE;
			return TRUE;
		}

		case EF_RESIST_ELEC:
		{
			if (player_inc_timed(p_ptr, TMD_OPP_ELEC, randint1(10) + 10, TRUE, TRUE))
				*ident = TRUE;
			return TRUE;
		}

		case EF_RESIST_FIRE:
		{
			if (player_inc_timed(p_ptr, TMD_OPP_FIRE, randint1(10) + 10, TRUE, TRUE))
				*ident = TRUE;
			return TRUE;
		}

		case EF_RESIST_COLD:
		{
			if (player_inc_timed(p_ptr, TMD_OPP_COLD, randint1(10) + 10, TRUE, TRUE))
				*ident = TRUE;
			return TRUE;
		}

		case EF_RESIST_POIS:
		{
			if (player_inc_timed(p_ptr, TMD_OPP_POIS, randint1(10) + 10, TRUE, TRUE))
				*ident = TRUE;
			return TRUE;
		}

		case EF_RESIST_ALL:
		{
			if (player_inc_timed(p_ptr, TMD_OPP_ACID, randint1(20) + 20, TRUE, TRUE))
				*ident = TRUE;
			if (player_inc_timed(p_ptr, TMD_OPP_ELEC, randint1(20) + 20, TRUE, TRUE))
				*ident = TRUE;
			if (player_inc_timed(p_ptr, TMD_OPP_FIRE, randint1(20) + 20, TRUE, TRUE))
				*ident = TRUE;
			if (player_inc_timed(p_ptr, TMD_OPP_COLD, randint1(20) + 20, TRUE, TRUE))
				*ident = TRUE;
			if (player_inc_timed(p_ptr, TMD_OPP_POIS, randint1(20) + 20, TRUE, TRUE))
				*ident = TRUE;
			return TRUE;
		}

		case EF_DETECT_TREASURE:
		{
			if (detect_treasure(aware)) *ident = TRUE;
			return TRUE;
		}

		case EF_DETECT_TRAP:
		{
			if (detect_traps(aware)) *ident = TRUE;
			return TRUE;
		}

		case EF_DETECT_DOORSTAIR:
		{
			if (detect_doorstairs(aware)) *ident = TRUE;
			return TRUE;
		}

		case EF_DETECT_INVIS:
		{
			if (detect_monsters_invis(aware)) *ident = TRUE;
			return TRUE;
		}

		case EF_DETECT_EVIL:
		{
			if (detect_monsters_evil(aware)) *ident = TRUE;
			return TRUE;
		}

		case EF_DETECT_ALL:
		{
			if (detect_all(aware)) *ident = TRUE;
			return TRUE;
		}

		case EF_ENCHANT_TOHIT:
		{
			*ident = TRUE;
			return enchant_spell(1, 0, 0);
		}

		case EF_ENCHANT_TODAM:
		{
			*ident = TRUE;
			return enchant_spell(0, 1, 0);
		}

		case EF_ENCHANT_WEAPON:
		{
			*ident = TRUE;
			return enchant_spell(randint1(3), randint1(3), 0);
		}

		case EF_ENCHANT_ARMOR:
		{
			*ident = TRUE;
			return enchant_spell(0, 0, 1);
		}

		case EF_ENCHANT_ARMOR2:
		{
			*ident = TRUE;
			return enchant_spell(0, 0, randint1(3) + 2);
		}

		case EF_RESTORE_ITEM:
		{
			*ident = TRUE;
			return restore_item();
		}

		case EF_IDENTIFY:
		{
			*ident = TRUE;
			if (!ident_spell()) return FALSE;
			return TRUE;
		}

		case EF_REMOVE_CURSE:
		{
			if (remove_curse())
			{
				if (!p_ptr->timed[TMD_BLIND])
					msg("The air around your body glows blue for a moment...");
				else
					msg("You feel as if someone is watching over you.");

				*ident = TRUE;
			}
			return TRUE;
		}

		case EF_REMOVE_CURSE2:
		{
			remove_all_curse();
			*ident = TRUE;
			return TRUE;
		}

		case EF_LIGHT:
		{
			if (light_area(damroll(2, 8), 2)) *ident = TRUE;
			return TRUE;
		}

		case EF_SUMMON_MON:
		{
			int i;
			sound(MSG_SUM_MONSTER);

			for (i = 0; i < randint1(3); i++)
			{
				if (summon_specific(py, px, p_ptr->depth, 0, 1))
					*ident = TRUE;
			}
			return TRUE;
		}

		case EF_SUMMON_UNDEAD:
		{
			int i;
			sound(MSG_SUM_UNDEAD);

			for (i = 0; i < randint1(3); i++)
			{
				if (summon_specific(py, px, p_ptr->depth,
					S_UNDEAD, 1))
					*ident = TRUE;
			}
			return TRUE;
		}

		case EF_TELE_PHASE:
		{
			teleport_player(10);
			*ident = TRUE;
			return TRUE;
		}

		case EF_TELE_LONG:
		{
			teleport_player(100);
			*ident = TRUE;
			return TRUE;
		}

		case EF_TELE_LEVEL:
		{
			(void)teleport_player_level();
			*ident = TRUE;
			return TRUE;
		}

		case EF_CONFUSING:
		{
			if (p_ptr->confusing == 0)
			{
				msg("Your hands begin to glow.");
				p_ptr->confusing = TRUE;
				*ident = TRUE;
			}
			return TRUE;
		}

		case EF_MAPPING:
		{
			map_area();
			*ident = TRUE;
			return TRUE;
		}

		case EF_RUNE:
		{
			warding_glyph();
			*ident = TRUE;
			return TRUE;
		}

		case EF_ACQUIRE:
		{
			acquirement(py, px, p_ptr->depth, 1, TRUE);
			*ident = TRUE;
			return TRUE;
		}

		case EF_ACQUIRE2:
		{
			acquirement(py, px, p_ptr->depth, randint1(2) + 1,
				TRUE);
			*ident = TRUE;
			return TRUE;
		}

		case EF_ANNOY_MON:
		{
			msg("There is a high pitched humming noise.");
			aggravate_monsters(0);
			*ident = TRUE;
			return TRUE;
		}

		case EF_CREATE_TRAP:
		{
			/* Hack -- no traps in the town */
			if (p_ptr->depth == 0)
				return TRUE;

			trap_creation();
			msg("You hear a low-pitched whistling sound.");
			*ident = TRUE;
			return TRUE;
		}

		case EF_DESTROY_TDOORS:
		{
			if (destroy_doors_touch()) *ident = TRUE;
			return TRUE;
		}

		case EF_RECHARGE:
		{
			*ident = TRUE;
			if (!recharge(60)) return FALSE;
			return TRUE;
		}

		case EF_BANISHMENT:
		{
			*ident = TRUE;
			if (!banishment()) return FALSE;
			return TRUE;
		}

		case EF_DARKNESS:
		{
			if (!check_state(p_ptr, OF_RES_DARK, p_ptr->state.flags))
				(void)player_inc_timed(p_ptr, TMD_BLIND, 3 + randint1(5), TRUE, TRUE);
			unlight_area(10, 3);
			wieldeds_notice_flag(p_ptr, OF_RES_DARK);
			*ident = TRUE;
			return TRUE;
		}

		case EF_PROTEVIL:
		{
			if (player_inc_timed(p_ptr, TMD_PROTEVIL, randint1(25) + 3 *
				p_ptr->lev, TRUE, TRUE)) *ident = TRUE;
			return TRUE;
		}

		case EF_SATISFY:
		{
			if (player_set_food(p_ptr, PY_FOOD_MAX - 1)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURSE_WEAPON:
		{
			if (curse_weapon()) *ident = TRUE;
			return TRUE;
		}

		case EF_CURSE_ARMOR:
		{
			if (curse_armor()) *ident = TRUE;
			return TRUE;
		}

		case EF_BLESSING:
		{
			if (player_inc_timed(p_ptr, TMD_BLESSED, randint1(12) + 6, TRUE, TRUE))
				*ident = TRUE;
			return TRUE;
		}

		case EF_BLESSING2:
		{
			if (player_inc_timed(p_ptr, TMD_BLESSED, randint1(24) + 12, TRUE, TRUE))
				*ident = TRUE;
			return TRUE;
		}

		case EF_BLESSING3:
		{
			if (player_inc_timed(p_ptr, TMD_BLESSED, randint1(48) + 24, TRUE, TRUE))
				*ident = TRUE;
			return TRUE;
		}

		case EF_RECALL:
		{
			set_recall();
			*ident = TRUE;
			return TRUE;
		}

		case EF_DEEP_DESCENT:
		{
			int i, target_depth = p_ptr->depth;
			
			/* Calculate target depth */
			for (i = 2; i > 0; i--) {
				if (is_quest(target_depth)) break;
				if (target_depth >= MAX_DEPTH - 1) break;
				
				target_depth++;
			}

			if (target_depth > p_ptr->depth) {
				msgt(MSG_TPLEVEL, "You sink through the floor...");
				dungeon_change_level(target_depth);
				*ident = TRUE;
				return TRUE;
			} else {
				msgt(MSG_TPLEVEL, "You sense a malevolent presence blocking passage to the levels below.");
				*ident = TRUE;
				return FALSE;
			}
		}

		case EF_LOSHASTE:
		{
			if (speed_monsters()) *ident = TRUE;
			return TRUE;
		}

		case EF_LOSSLEEP:
		{
			if (sleep_monsters(aware)) *ident = TRUE;
			return TRUE;
		}

		case EF_LOSSLOW:
		{
			if (slow_monsters()) *ident = TRUE;
			return TRUE;
		}

		case EF_LOSCONF:
		{
			if (confuse_monsters(aware)) *ident = TRUE;
			return TRUE;
		}

		case EF_LOSKILL:
		{
			(void)mass_banishment();
			*ident = TRUE;
			return TRUE;
		}

		case EF_EARTHQUAKES:
		{
			earthquake(py, px, 10);
			*ident = TRUE;
			return TRUE;
		}

		case EF_DESTRUCTION2:
		{
			destroy_area(py, px, 15, TRUE);
			*ident = TRUE;
			return TRUE;
		}

		case EF_ILLUMINATION:
		{
			if (light_area(damroll(2, 15), 3)) *ident = TRUE;
			return TRUE;
		}

		case EF_CLAIRVOYANCE:
		{
			*ident = TRUE;
			wiz_light();
			(void)detect_traps(TRUE);
			(void)detect_doorstairs(TRUE);
			return TRUE;
		}

		case EF_PROBING:
		{
			*ident = probing();
			return TRUE;
		}

		case EF_STONE_TO_MUD:
		{
			if (wall_to_mud(dir)) *ident = TRUE;
			return TRUE;
		}

		case EF_CONFUSE2:
		{
			*ident = TRUE;
			confuse_monster(dir, 20, aware);
			return TRUE;
		}

		case EF_BIZARRE:
		{
			*ident = TRUE;
			ring_of_power(dir);
			return TRUE;
		}

		case EF_STAR_BALL:
		{
			int i;
			*ident = TRUE;
			for (i = 0; i < 8; i++) fire_ball(GF_ELEC, ddd[i],
				(150 * (100 + boost) / 100), 3);
			return TRUE;
		}

		case EF_RAGE_BLESS_RESIST:
		{
			dur = randint1(50) + 50;
			*ident = TRUE;
			(void)hp_player(30);
			(void)player_clear_timed(p_ptr, TMD_AFRAID, TRUE);
			(void)player_inc_timed(p_ptr, TMD_BOLD, dur, TRUE, TRUE);
			(void)player_inc_timed(p_ptr, TMD_SHERO, dur, TRUE, TRUE);
			(void)player_inc_timed(p_ptr, TMD_BLESSED, randint1(50) + 50, TRUE, TRUE);
			(void)player_inc_timed(p_ptr, TMD_OPP_ACID, randint1(50) + 50, TRUE, TRUE);
			(void)player_inc_timed(p_ptr, TMD_OPP_ELEC, randint1(50) + 50, TRUE, TRUE);
			(void)player_inc_timed(p_ptr, TMD_OPP_FIRE, randint1(50) + 50, TRUE, TRUE);
			(void)player_inc_timed(p_ptr, TMD_OPP_COLD, randint1(50) + 50, TRUE, TRUE);
			(void)player_inc_timed(p_ptr, TMD_OPP_POIS, randint1(50) + 50, TRUE, TRUE);
			return TRUE;
		}

		case EF_SLEEPII:
		{
			*ident = TRUE;
			sleep_monsters_touch(aware);
			return TRUE;
		}

		case EF_RESTORE_LIFE:
		{
			*ident = TRUE;
			restore_level();
			return TRUE;
		}

		case EF_MISSILE:
		{
			*ident = TRUE;
			dam = damroll(3, 4) * (100 + boost) / 100;
			fire_bolt_or_beam(beam, GF_MISSILE, dir, dam);
			return TRUE;
		}

		case EF_DISPEL_EVIL:
		{
			*ident = TRUE;
			dam = p_ptr->lev * 5 * (100 + boost) / 100;
			dispel_evil(dam);
			return TRUE;
		}

		case EF_DISPEL_EVIL60:
		{
			dam = 60 * (100 + boost) / 100;
			if (dispel_evil(dam)) *ident = TRUE;
			return TRUE;
		}

		case EF_DISPEL_UNDEAD:
		{
			dam = 60 * (100 + boost) / 100;
			if (dispel_undead(dam)) *ident = TRUE;
			return TRUE;
		}

		case EF_DISPEL_ALL:
		{
			dam = 120 * (100 + boost) / 100;
			if (dispel_monsters(dam)) *ident = TRUE;
			return TRUE;
		}

		case EF_HASTE:
		{
			if (!p_ptr->timed[TMD_FAST])
			{
				if (player_set_timed(p_ptr, TMD_FAST, damroll(2, 10) + 20, TRUE)) *ident = TRUE;
			}
			else
			{
				(void)player_inc_timed(p_ptr, TMD_FAST, 5, TRUE, TRUE);
			}

			return TRUE;
		}

		case EF_HASTE1:
		{
			if (!p_ptr->timed[TMD_FAST])
			{
				if (player_set_timed(p_ptr, TMD_FAST, randint1(20) + 20, TRUE)) *ident = TRUE;
			}
			else
			{
				(void)player_inc_timed(p_ptr, TMD_FAST, 5, TRUE, TRUE);
			}

			return TRUE;
		}

		case EF_HASTE2:
		{
			if (!p_ptr->timed[TMD_FAST])
			{
				if (player_set_timed(p_ptr, TMD_FAST, randint1(75) + 75, TRUE)) *ident = TRUE;
			}
			else
			{
				(void)player_inc_timed(p_ptr, TMD_FAST, 5, TRUE, TRUE);
			}

			return TRUE;
		}


		case EF_FIRE_BOLT:
		{
			*ident = TRUE;
			dam = damroll(9, 8) * (100 + boost) / 100;
			fire_bolt(GF_FIRE, dir, dam);
			return TRUE;
		}

		case EF_FIRE_BOLT2:
		{
			dam = damroll(12, 8) * (100 + boost) / 100;
			fire_bolt_or_beam(beam, GF_FIRE, dir, dam);
			*ident = TRUE;
			return TRUE;
		}

		case EF_FIRE_BOLT3:
		{
			dam = damroll(16, 8) * (100 + boost) / 100;
			fire_bolt_or_beam(beam, GF_FIRE, dir, dam);
			*ident = TRUE;
			return TRUE;
		}

		case EF_FIRE_BOLT72:
		{
			dam = 72 * (100 + boost) / 100;
			*ident = TRUE;
			fire_ball(GF_FIRE, dir, dam, 2);
			return TRUE;
		}

		case EF_FIRE_BALL:
		{
			dam = 144 * (100 + boost) / 100;
			fire_ball(GF_FIRE, dir, dam, 2);
			*ident = TRUE;
			return TRUE;
		}

		case EF_FIRE_BALL2:
		{
			dam = 120 * (100 + boost) / 100;
			*ident = TRUE;
			fire_ball(GF_FIRE, dir, dam, 3);
			return TRUE;
		}

		case EF_FIRE_BALL200:
		{
			dam = 200 * (100 + boost) / 100;
			*ident = TRUE;
			fire_ball(GF_FIRE, dir, dam, 3);
			return TRUE;
		}

		case EF_COLD_BOLT:
		{
			dam = damroll(6, 8) * (100 + boost) / 100;
			*ident = TRUE;
			fire_bolt_or_beam(beam, GF_COLD, dir, dam);
			return TRUE;
		}

		case EF_COLD_BOLT2:
		{
			dam = damroll(12, 8) * (100 + boost) / 100;
			*ident = TRUE;
			fire_bolt(GF_COLD, dir, dam);
			return TRUE;
		}

		case EF_COLD_BALL2:
		{
			dam = 200 * (100 + boost) / 100;
			*ident = TRUE;
			fire_ball(GF_COLD, dir, dam, 3);
			return TRUE;
		}

		case EF_COLD_BALL50:
		{
			dam = 50 * (100 + boost) / 100;
			*ident = TRUE;
			fire_ball(GF_COLD, dir, dam, 2);
			return TRUE;
		}

		case EF_COLD_BALL100:
		{
			dam = 100 * (100 + boost) / 100;
			*ident = TRUE;
			fire_ball(GF_COLD, dir, dam, 2);
			return TRUE;
		}

		case EF_COLD_BALL160:
		{
			dam = 160 * (100 + boost) / 100;
			*ident = TRUE;
			fire_ball(GF_COLD, dir, dam, 3);
			return TRUE;
		}

		case EF_ACID_BOLT:
		{
			dam = damroll(5, 8) * (100 + boost) / 100;
			*ident = TRUE;
			fire_bolt(GF_ACID, dir, dam);
			return TRUE;
		}

		case EF_ACID_BOLT2:
		{
			dam = damroll(10, 8) * (100 + boost) / 100;
			fire_bolt_or_beam(beam, GF_ACID, dir, dam);
			*ident = TRUE;
			return TRUE;
		}

		case EF_ACID_BOLT3:
		{
			dam = damroll(12, 8) * (100 + boost) / 100;
			fire_bolt_or_beam(beam, GF_ACID, dir, dam);
			*ident = TRUE;
			return TRUE;
		}

		case EF_ACID_BALL:
		{
			dam = 120 * (100 + boost) / 100;
			fire_ball(GF_ACID, dir, dam, 2);
			*ident = TRUE;
			return TRUE;
		}

		case EF_ELEC_BOLT:
		{
			dam = damroll(6, 6) * (100 + boost) / 100;
			*ident = TRUE;
			fire_beam(GF_ELEC, dir, dam);
			return TRUE;
		}

		case EF_ELEC_BALL:
		{
			dam = 64 * (100 + boost) / 100;
			fire_ball(GF_ELEC, dir, dam, 2);
			*ident = TRUE;
			return TRUE;
		}

		case EF_ELEC_BALL2:
		{
			dam = 250 * (100 + boost) / 100;
			*ident = TRUE;
			fire_ball(GF_ELEC, dir, dam, 3);
			return TRUE;
		}


		case EF_ARROW:
		{
			dam = 150 * (100 + boost) / 100;
			*ident = TRUE;
			fire_bolt(GF_ARROW, dir, dam);
			return TRUE;
		}

		case EF_REM_FEAR_POIS:
		{
			*ident = TRUE;
			(void)player_clear_timed(p_ptr, TMD_AFRAID, TRUE);
			(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
			return TRUE;
		}

		case EF_STINKING_CLOUD:
		{
			dam = 12 * (100 + boost) / 100;
			*ident = TRUE;
			fire_ball(GF_POIS, dir, dam, 3);
			return TRUE;
		}


		case EF_DRAIN_LIFE1:
		{
			dam = 90 * (100 + boost) / 100;
			if (drain_life(dir, dam)) *ident = TRUE;
			return TRUE;
		}

		case EF_DRAIN_LIFE2:
		{
			dam = 120 * (100 + boost) / 100;
			if (drain_life(dir, dam)) *ident = TRUE;
			return TRUE;
		}

		case EF_DRAIN_LIFE3:
		{
			dam = 150 * (100 + boost) / 100;
			if (drain_life(dir, dam)) *ident = TRUE;
			return TRUE;
		}

		case EF_DRAIN_LIFE4:
		{
			dam = 250 * (100 + boost) / 100;
			if (drain_life(dir, dam)) *ident = TRUE;
			return TRUE;
		}

		case EF_FIREBRAND:
		{
			*ident = TRUE;
			if (!brand_bolts()) return FALSE;
			return TRUE;
		}

		case EF_MANA_BOLT:
		{
			dam = damroll(12, 8) * (100 + boost) / 100;
			fire_bolt(GF_MANA, dir, dam);
			*ident = TRUE;
			return TRUE;
		}

		case EF_MON_HEAL:
		{
			if (heal_monster(dir)) *ident = TRUE;
			return TRUE;
		}

		case EF_MON_HASTE:
		{
			if (speed_monster(dir)) *ident = TRUE;
			return TRUE;
		}

		case EF_MON_SLOW:
		{
			if (slow_monster(dir)) *ident = TRUE;
			return TRUE;
		}

		case EF_MON_CONFUSE:
		{
			if (confuse_monster(dir, 10, aware)) *ident = TRUE;
			return TRUE;
		}

		case EF_MON_SLEEP:
		{
			if (sleep_monster(dir, aware)) *ident = TRUE;
			return TRUE;
		}

		case EF_MON_CLONE:
		{
			if (clone_monster(dir)) *ident = TRUE;
			return TRUE;
		}

		case EF_MON_SCARE:
		{
			if (fear_monster(dir, 10, aware)) *ident = TRUE;
			return TRUE;
		}

		case EF_LIGHT_LINE:
		{
			msg("A line of shimmering blue light appears.");
			light_line(dir);
			*ident = TRUE;
			return TRUE;
		}

		case EF_TELE_OTHER:
		{
			if (teleport_monster(dir)) *ident = TRUE;
			return TRUE;
		}

		case EF_DISARMING:
		{
			if (disarm_trap(dir)) *ident = TRUE;
			return TRUE;
		}

		case EF_TDOOR_DEST:
		{
			if (destroy_door(dir)) *ident = TRUE;
			return TRUE;
		}

		case EF_POLYMORPH:
		{
			if (poly_monster(dir)) *ident = TRUE;
			return TRUE;
		}

		case EF_STARLIGHT:
		{
			int i;
			if (!p_ptr->timed[TMD_BLIND])
				msg("Light shoots in all directions!");
			for (i = 0; i < 8; i++) light_line(ddd[i]);
			*ident = TRUE;
			return TRUE;
		}

		case EF_STARLIGHT2:
		{
			int k;
			for (k = 0; k < 8; k++) strong_light_line(ddd[k]);
			*ident = TRUE;
			return TRUE;
		}

		case EF_BERSERKER:
		{
			dur = randint1(50) + 50;
			if (player_inc_timed(p_ptr, TMD_BOLD, dur, TRUE, TRUE)) *ident = TRUE;
			if (player_inc_timed(p_ptr, TMD_SHERO, dur, TRUE, TRUE)) *ident = TRUE;
			return TRUE;
		}

		case EF_WONDER:
		{
			if (effect_wonder(dir, randint1(100) + p_ptr->lev / 5,
				beam)) *ident = TRUE;
			return TRUE;
		}

		case EF_WAND_BREATH:
		{
			/* table of random ball effects and their damages */
			const int breath_types[] = {
				GF_ACID, 200,
				GF_ELEC, 160,
				GF_FIRE, 200,
				GF_COLD, 160,
				GF_POIS, 120
			};
			/* pick a random (type, damage) tuple in the table */
			int which = 2 * randint0(sizeof(breath_types) / (2 * sizeof(int)));
			fire_ball(breath_types[which], dir, breath_types[which + 1], 3);
			*ident = TRUE;
			return TRUE;
		}

		case EF_STAFF_MAGI:
		{
			if (do_res_stat(A_INT)) *ident = TRUE;
			if (p_ptr->csp < p_ptr->msp)
			{
				p_ptr->csp = p_ptr->msp;
				p_ptr->csp_frac = 0;
				*ident = TRUE;
				msg("Your feel your head clear.");
				p_ptr->redraw |= (PR_MANA);
			}
			return TRUE;
		}

		case EF_STAFF_HOLY:
		{
			dam = 120 * (100 + boost) / 100;
			if (dispel_evil(dam)) *ident = TRUE;
			if (player_inc_timed(p_ptr, TMD_PROTEVIL, randint1(25) + 3 *
				p_ptr->lev, TRUE, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_POISONED, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_AFRAID, TRUE)) *ident = TRUE;
			if (hp_player(50)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_STUN, TRUE)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_CUT, TRUE)) *ident = TRUE;
			return TRUE;
		}

		case EF_DRINK_BREATH:
		{
			const int breath_types[] =
			{
				GF_FIRE, 80,
				GF_COLD, 80,
			};

			int which = 2 * randint0(N_ELEMENTS(breath_types) / 2);
			fire_ball(breath_types[which], dir, breath_types[which + 1], 2);
			*ident = TRUE;
			return TRUE;
		}

		case EF_DRINK_GOOD:
		{
			msg("You feel less thirsty.");
			*ident = TRUE;
			return TRUE;
		}

		case EF_DRINK_DEATH:
		{
			msg("A feeling of Death flows through your body.");
			take_hit(p_ptr, 5000, "a potion of Death");
			*ident = TRUE;
			return TRUE;
		}

		case EF_DRINK_RUIN:
		{
			msg("Your nerves and muscles feel weak and lifeless!");
			take_hit(p_ptr, damroll(10, 10), "a potion of Ruination");
			player_stat_dec(p_ptr, A_DEX, TRUE);
			player_stat_dec(p_ptr, A_WIS, TRUE);
			player_stat_dec(p_ptr, A_CON, TRUE);
			player_stat_dec(p_ptr, A_STR, TRUE);
			player_stat_dec(p_ptr, A_CHR, TRUE);
			player_stat_dec(p_ptr, A_INT, TRUE);
			*ident = TRUE;
			return TRUE;
		}

		case EF_DRINK_DETONATE:
		{
			msg("Massive explosions rupture your body!");
			take_hit(p_ptr, damroll(50, 20), "a potion of Detonation");
			(void)player_inc_timed(p_ptr, TMD_STUN, 75, TRUE, TRUE);
			(void)player_inc_timed(p_ptr, TMD_CUT, 5000, TRUE, TRUE);
			*ident = TRUE;
			return TRUE;
		}

		case EF_DRINK_SALT:
		{
			msg("The potion makes you vomit!");
			player_set_food(p_ptr, PY_FOOD_STARVE - 1);
			(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
			(void)player_inc_timed(p_ptr, TMD_PARALYZED, 4, TRUE, FALSE);
			*ident = TRUE;
			return TRUE;
		}

		case EF_FOOD_GOOD:
		{
			msg("That tastes good.");
			*ident = TRUE;
			return TRUE;
		}

		case EF_FOOD_WAYBREAD:
		{
			msg("That tastes good.");
			(void)player_clear_timed(p_ptr, TMD_POISONED, TRUE);
			(void)hp_player(damroll(4, 8));
			*ident = TRUE;
			return TRUE;
		}

		case EF_SHROOM_EMERGENCY:
		{
			(void)player_set_timed(p_ptr, TMD_IMAGE, rand_spread(250, 50), TRUE);
			(void)player_set_timed(p_ptr, TMD_OPP_FIRE, rand_spread(30, 10), TRUE);
			(void)player_set_timed(p_ptr, TMD_OPP_COLD, rand_spread(30, 10), TRUE);
			(void)hp_player(200);
			*ident = TRUE;
			return TRUE;
		}

		case EF_SHROOM_TERROR:
		{
			if (player_set_timed(p_ptr, TMD_TERROR, rand_spread(100, 20), TRUE))
				*ident = TRUE;
			return TRUE;
		}

		case EF_SHROOM_STONE:
		{
			if (player_set_timed(p_ptr, TMD_STONESKIN, rand_spread(80, 20), TRUE))
				*ident = TRUE;
			return TRUE;
		}

		case EF_SHROOM_DEBILITY:
		{
			int stat = one_in_(2) ? A_STR : A_CON;

			if (p_ptr->csp < p_ptr->msp)
			{
				p_ptr->csp = p_ptr->msp;
				p_ptr->csp_frac = 0;
				msg("Your feel your head clear.");
				p_ptr->redraw |= (PR_MANA);
				*ident = TRUE;
			}

			(void)do_dec_stat(stat, FALSE);

			*ident = TRUE;
			return TRUE;
		}

		case EF_SHROOM_SPRINTING:
		{
			if (player_inc_timed(p_ptr, TMD_SPRINT, 100, TRUE, TRUE)) *ident = TRUE;
			return TRUE;
		}

		case EF_SHROOM_PURGING:
		{
			player_set_food(p_ptr, PY_FOOD_FAINT - 1);
			if (do_res_stat(A_STR)) *ident = TRUE;
			if (do_res_stat(A_CON)) *ident = TRUE;
			if (player_clear_timed(p_ptr, TMD_POISONED, TRUE)) *ident = TRUE;
			return TRUE;
		}

		case EF_RING_ACID:
		{
			dam = 70 * (100 + boost) / 100;
			*ident = TRUE;
			fire_ball(GF_ACID, dir, dam, 2);
			player_inc_timed(p_ptr, TMD_OPP_ACID, randint1(20) + 20, TRUE, TRUE);
			return TRUE;
		}

		case EF_RING_FLAMES:
		{
			dam = 80 * (100 + boost) / 100;
			*ident = TRUE;
			fire_ball(GF_FIRE, dir, dam, 2);
			player_inc_timed(p_ptr, TMD_OPP_FIRE, randint1(20) + 20, TRUE, TRUE);
			return TRUE;
		}

		case EF_RING_ICE:
		{
			dam = 75 * (100 + boost) / 100;
			*ident = TRUE;
			fire_ball(GF_COLD, dir, dam, 2);
			player_inc_timed(p_ptr, TMD_OPP_COLD, randint1(20) + 20, TRUE, TRUE);
			return TRUE;
		}

		case EF_RING_LIGHTNING:
		{
			dam = 85 * (100 + boost) / 100;
			*ident = TRUE;
			fire_ball(GF_ELEC, dir, dam, 2);
			player_inc_timed(p_ptr, TMD_OPP_ELEC, randint1(20) + 20, TRUE, TRUE);
			return TRUE;
		}

		case EF_DRAGON_BLUE:
		{
			dam = 100 * (100 + boost) / 100;
			msgt(MSG_BR_ELEC, "You breathe lightning.");
			fire_ball(GF_ELEC, dir, dam, 2);
			return TRUE;
		}

		case EF_DRAGON_GREEN:
		{
			dam = 150 * (100 + boost) / 100;
			msgt(MSG_BR_GAS, "You breathe poison gas.");
			fire_ball(GF_POIS, dir, dam, 2);
			return TRUE;
		}

		case EF_DRAGON_RED:
		{
			dam = 200 * (100 + boost) / 100;
			msgt(MSG_BR_FIRE, "You breathe fire.");
			fire_ball(GF_FIRE, dir, dam, 2);
			return TRUE;
		}

		case EF_DRAGON_MULTIHUED:
		{
			static const struct
			{
				int msg_sound;
				const char *msg;
				int typ;
			} mh[] =
			{
				{ MSG_BR_ELEC,  "lightning",  GF_ELEC },
				{ MSG_BR_FROST, "frost",      GF_COLD },
				{ MSG_BR_ACID,  "acid",       GF_ACID },
				{ MSG_BR_GAS,   "poison gas", GF_POIS },
				{ MSG_BR_FIRE,  "fire",       GF_FIRE }
			};

			int chance = randint0(5);
			dam = 250 * (100 + boost) / 100;
			msgt(mh[chance].msg_sound, "You breathe %s.", mh[chance].msg);
			fire_ball(mh[chance].typ, dir, dam, 2);
			return TRUE;
		}

		case EF_DRAGON_BRONZE:
		{
			dam = 120 * (100 + boost) / 100;
			msgt(MSG_BR_CONF, "You breathe confusion.");
			fire_ball(GF_CONFU, dir, dam, 2);
			return TRUE;
		}

		case EF_DRAGON_GOLD:
		{
			dam = 130 * (100 + boost) / 100;
			msgt(MSG_BR_SOUND, "You breathe sound.");
			fire_ball(GF_SOUND, dir, dam, 2);
			return TRUE;
		}

		case EF_DRAGON_CHAOS:
		{
			dam = 220 * (100 + boost) / 100;
			chance = randint0(2);
			msgt((chance == 1 ? MSG_BR_CHAOS : MSG_BR_DISEN),
					"You breathe %s.",
					((chance == 1 ? "chaos" : "disenchantment")));
			fire_ball((chance == 1 ? GF_CHAOS : GF_DISEN),
			          dir, dam, 2);
			return TRUE;
		}

		case EF_DRAGON_LAW:
		{
			dam = 230 * (100 + boost) / 100;
			chance = randint0(2);
			msgt((chance == 1 ? MSG_BR_SOUND : MSG_BR_SHARDS), "You breathe %s.",
			           ((chance == 1 ? "sound" : "shards")));
			fire_ball((chance == 1 ? GF_SOUND : GF_SHARD),
			          dir, dam, 2);
			return TRUE;
		}

		case EF_DRAGON_BALANCE:
		{
			dam = 250 * (100 + boost) / 100;
			chance = randint0(4);
			msg("You breathe %s.",
			           ((chance == 1) ? "chaos" :
			            ((chance == 2) ? "disenchantment" :
			             ((chance == 3) ? "sound" : "shards"))));
			fire_ball(((chance == 1) ? GF_CHAOS :
			           ((chance == 2) ? GF_DISEN :
			            ((chance == 3) ? GF_SOUND : GF_SHARD))),
			          dir, dam, 2);
			return TRUE;
		}

		case EF_DRAGON_SHINING:
		{
			dam = 200 * (100 + boost) / 100;
			chance = randint0(2);
			msgt((chance == 0 ? MSG_BR_LIGHT : MSG_BR_DARK), "You breathe %s.",
			        ((chance == 0 ? "light" : "darkness")));
			fire_ball((chance == 0 ? GF_LIGHT : GF_DARK), dir, dam,
				2);
			return TRUE;
		}

		case EF_DRAGON_POWER:
		{
			dam = 300 * (100 + boost) / 100;
			msgt(MSG_BR_ELEMENTS, "You breathe the elements.");
			fire_ball(GF_MISSILE, dir, dam, 2);
			return TRUE;
		}

		case EF_TRAP_DOOR:
		{
			msg("You fall through a trap door!");
			if (check_state(p_ptr, OF_FEATHER, p_ptr->state.flags)) {
				msg("You float gently down to the next level.");
			} else {
				take_hit(p_ptr, damroll(2, 8), "a trap");
			}
			wieldeds_notice_flag(p_ptr, OF_FEATHER);

			dungeon_change_level(p_ptr->depth + 1);
			return TRUE;
		}

		case EF_TRAP_PIT:
		{
			msg("You fall into a pit!");
			if (check_state(p_ptr, OF_FEATHER, p_ptr->state.flags)) {
				msg("You float gently to the bottom of the pit.");
			} else {
				take_hit(p_ptr, damroll(2, 6), "a trap");
			}
			wieldeds_notice_flag(p_ptr, OF_FEATHER);
			return TRUE;
		}

		case EF_TRAP_PIT_SPIKES:
		{
			msg("You fall into a spiked pit!");

			if (check_state(p_ptr, OF_FEATHER, p_ptr->state.flags)) {
				msg("You float gently to the floor of the pit.");
				msg("You carefully avoid touching the spikes.");
			} else {
				int dam = damroll(2, 6);

				/* Extra spike damage */
				if (one_in_(2)) {
					msg("You are impaled!");
					dam *= 2;
					(void)player_inc_timed(p_ptr, TMD_CUT, randint1(dam), TRUE, TRUE);
				}

				take_hit(p_ptr, dam, "a trap");
			}
			wieldeds_notice_flag(p_ptr, OF_FEATHER);
			return TRUE;
		}

		case EF_TRAP_PIT_POISON:
		{
			msg("You fall into a spiked pit!");

			if (check_state(p_ptr, OF_FEATHER, p_ptr->state.flags)) {
				msg("You float gently to the floor of the pit.");
				msg("You carefully avoid touching the spikes.");
			} else {
				int dam = damroll(2, 6);

				/* Extra spike damage */
				if (one_in_(2)) {
					msg("You are impaled on poisonous spikes!");
					(void)player_inc_timed(p_ptr, TMD_CUT, randint1(dam * 2), TRUE, TRUE);
					(void)player_inc_timed(p_ptr, TMD_POISONED, randint1(dam * 4), TRUE, TRUE);
				}

				take_hit(p_ptr, dam, "a trap");
			}
			wieldeds_notice_flag(p_ptr, OF_FEATHER);
			return TRUE;
		}

		case EF_TRAP_RUNE_SUMMON:
		{
			int i;
			int num = 2 + randint1(3);

			msgt(MSG_SUM_MONSTER, "You are enveloped in a cloud of smoke!");

			/* Remove trap */
			cave->info[py][px] &= ~(CAVE_MARK);
			cave_set_feat(cave, py, px, FEAT_FLOOR);

			for (i = 0; i < num; i++)
				(void)summon_specific(py, px, p_ptr->depth, 0, 1);

			return TRUE;
		}

		case EF_TRAP_RUNE_TELEPORT:
		{
			msg("You hit a teleport trap!");
			teleport_player(100);
			return TRUE;
		}

		case EF_TRAP_SPOT_FIRE:
		{
			int dam;

			msg("You are enveloped in flames!");
			dam = damroll(4, 6);
			dam = adjust_dam(p_ptr, GF_FIRE, dam, RANDOMISE,
					check_for_resist(p_ptr, GF_FIRE, p_ptr->state.flags, TRUE));
			if (dam) {
				take_hit(p_ptr, dam, "a fire trap");
				inven_damage(p_ptr, GF_FIRE, MIN(dam * 5, 300));
			}
			return TRUE;
		}

		case EF_TRAP_SPOT_ACID:
		{
			int dam;

			msg("You are splashed with acid!");
			dam = damroll(4, 6);
			dam = adjust_dam(p_ptr, GF_ACID, dam, RANDOMISE,
					check_for_resist(p_ptr, GF_ACID, p_ptr->state.flags, TRUE));
			if (dam) {
				take_hit(p_ptr, dam, "an acid trap");
				inven_damage(p_ptr, GF_ACID, MIN(dam * 5, 300));
			}
			return TRUE;
		}

		case EF_TRAP_DART_SLOW:
		{
			if (trap_check_hit(125)) {
				msg("A small dart hits you!");
				take_hit(p_ptr, damroll(1, 4), "a trap");
				(void)player_inc_timed(p_ptr, TMD_SLOW, randint0(20) + 20, TRUE, FALSE);
			} else {
				msg("A small dart barely misses you.");
			}
			return TRUE;
		}

		case EF_TRAP_DART_LOSE_STR:
		{
			if (trap_check_hit(125)) {
				msg("A small dart hits you!");
				take_hit(p_ptr, damroll(1, 4), "a trap");
				(void)do_dec_stat(A_STR, FALSE);
			} else {
				msg("A small dart barely misses you.");
			}
			return TRUE;
		}

		case EF_TRAP_DART_LOSE_DEX:
		{
			if (trap_check_hit(125)) {
				msg("A small dart hits you!");
				take_hit(p_ptr, damroll(1, 4), "a trap");
				(void)do_dec_stat(A_DEX, FALSE);
			} else {
				msg("A small dart barely misses you.");
			}
			return TRUE;
		}

		case EF_TRAP_DART_LOSE_CON:
		{
			if (trap_check_hit(125)) {
				msg("A small dart hits you!");
				take_hit(p_ptr, damroll(1, 4), "a trap");
				(void)do_dec_stat(A_CON, FALSE);
			} else {
				msg("A small dart barely misses you.");
			}
			return TRUE;
		}

		case EF_TRAP_GAS_BLIND:
		{
			msg("You are surrounded by a black gas!");
			(void)player_inc_timed(p_ptr, TMD_BLIND, randint0(50) + 25, TRUE, TRUE);
			return TRUE;
		}

		case EF_TRAP_GAS_CONFUSE:
		{
			msg("You are surrounded by a gas of scintillating colors!");
			(void)player_inc_timed(p_ptr, TMD_CONFUSED, randint0(20) + 10, TRUE, TRUE);
			return TRUE;
		}

		case EF_TRAP_GAS_POISON:
		{
			msg("You are surrounded by a pungent green gas!");
			(void)player_inc_timed(p_ptr, TMD_POISONED, randint0(20) + 10, TRUE, TRUE);
			return TRUE;
		}

		case EF_TRAP_GAS_SLEEP:
		{
			msg("You are surrounded by a strange white mist!");
			(void)player_inc_timed(p_ptr, TMD_PARALYZED, randint0(10) + 5, TRUE, TRUE);
			return TRUE;
		}


		case EF_XXX:
		case EF_MAX:
			break;
	}

	/* Not used */
	msg("Effect not handled.");
	return FALSE;
}
