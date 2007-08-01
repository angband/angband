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
#include "effects.h"


/*
 * Entries for spell/activation descriptions
 */
typedef struct
{
	u16b index;          /* Effect index */
	bool aim;            /* Whether the effect requires aiming */
	const char *desc;    /* Effect description */
} info_entry;

/*
 * Useful things about effects.
 */
static const info_entry effects[] =
{
	#define EFFECT(x, y, z)	{ EF_##x, y, z },
	#include "effects.h"
};


/*
 * Utility functions
 */
bool effect_aim(int effect)
{
	if (effect < 1 || effect > EF_MAX)
		return FALSE;

	return effects[effect].aim;
}

const char *effect_desc(int effect)
{
	if (effect < 1 || effect > EF_MAX)
		return FALSE;

	return effects[effect].desc;
}


/*
 * The "wonder" effect.
 * 
 * Returns TRUE if the effect is evident.
 */
static bool spell_wonder(int dir, int die, int beam)
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
		msg_print("You feel a surge of power!");
		visible = TRUE;
	}

	if (die < 8) visible = clone_monster(dir);
	else if (die < 14) visible = speed_monster(dir);
	else if (die < 26) visible = heal_monster(dir);
	else if (die < 31) visible = poly_monster(dir);
	else if (die < 36)
		visible = fire_bolt_or_beam(beam - 10, GF_MISSILE, dir,
		                            damroll(3 + ((plev - 1) / 5), 4));
	else if (die < 41) visible = confuse_monster(dir, plev);
	else if (die < 46) visible = fire_ball(GF_POIS, dir, 20 + (plev / 2), 3);
	else if (die < 51) visible = lite_line(dir);
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
		sleep_monsters();
		hp_player(300);
	}

	return visible;
}




/*
 * Do an effect, given an object.
 */
bool do_effect(int effect, bool *ident, int dir, int beam)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	if (effect < 1 || effect > EF_MAX)
	{
		msg_print("Bad effect passed to do_effect().  Please report this bug.");
		return FALSE;
	}

	switch (effect)
	{
		case EF_POISON:
		{
			if (!(p_ptr->resist_pois || p_ptr->timed[TMD_OPP_POIS]))
			{
				if (inc_timed(TMD_POISONED, damroll(2, 7) + 10))
					*ident = TRUE;
			}

			return TRUE;
		}

		case EF_BLIND:
		{
			if (!p_ptr->resist_blind && inc_timed(TMD_BLIND, damroll(4, 25) + 75))
				*ident = TRUE;

			return TRUE;
		}

		case EF_SCARE:
		{
			if (!p_ptr->resist_fear && inc_timed(TMD_AFRAID, rand_int(10) + 10))
				*ident = TRUE;

			return TRUE;
		}

		case EF_CONFUSE:
		{
			if (!p_ptr->resist_confu && inc_timed(TMD_CONFUSED, damroll(4, 5) + 10))
				*ident = TRUE;

			return TRUE;
		}

		case EF_HALLUC:
		{
			if (!p_ptr->resist_chaos && inc_timed(TMD_IMAGE, rand_int(250) + 250))
				*ident = TRUE;

			return TRUE;
		}

		case EF_PARALYZE:
		{
			if (!p_ptr->free_act && inc_timed(TMD_PARALYZED, rand_int(5) + 5))
				*ident = TRUE;

			return TRUE;
		}

		case EF_SLOW:
		{
			if (inc_timed(TMD_SLOW, randint(25) + 15)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURE_POISON:
		{
			if (clear_timed(TMD_POISONED)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURE_BLINDNESS:
		{
			if (clear_timed(TMD_BLIND)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURE_PARANOIA:
		{
			if (clear_timed(TMD_AFRAID)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURE_CONFUSION:
		{
			if (clear_timed(TMD_CONFUSED)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURE_MIND:
		{
			if (clear_timed(TMD_CONFUSED)) *ident = TRUE;
			if (clear_timed(TMD_AFRAID)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURE_BODY:
		{
			if (clear_timed(TMD_STUN)) *ident = TRUE;
			if (clear_timed(TMD_CUT)) *ident = TRUE;
			if (clear_timed(TMD_POISONED)) *ident = TRUE;
			if (clear_timed(TMD_BLIND)) *ident = TRUE;
			return TRUE;
		}


		case EF_CURE_LIGHT:
		{
			if (heal_player(5, 10)) *ident = TRUE;
			if (clear_timed(TMD_BLIND)) *ident = TRUE;
			if (dec_timed(TMD_CUT, 10)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURE_SERIOUS:
		{
			if (heal_player(15, 21)) *ident = TRUE;
			if (set_timed(TMD_CUT, (p_ptr->timed[TMD_CUT] / 2) - 50)) *ident = TRUE;
			if (clear_timed(TMD_BLIND)) *ident = TRUE;
			if (clear_timed(TMD_CONFUSED)) *ident = TRUE;

			return TRUE;
		}

		case EF_CURE_CRITICAL:
		{
			if (heal_player(30, 30)) *ident = TRUE;
			if (clear_timed(TMD_BLIND)) *ident = TRUE;
			if (clear_timed(TMD_CONFUSED)) *ident = TRUE;
			if (clear_timed(TMD_POISONED)) *ident = TRUE;
			if (clear_timed(TMD_STUN)) *ident = TRUE;
			if (clear_timed(TMD_CUT)) *ident = TRUE;
			if (clear_timed(TMD_AMNESIA)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURE_FULL:
		{
			if (hp_player(300)) *ident = TRUE;
			if (clear_timed(TMD_BLIND)) *ident = TRUE;
			if (clear_timed(TMD_CONFUSED)) *ident = TRUE;
			if (clear_timed(TMD_POISONED)) *ident = TRUE;
			if (clear_timed(TMD_STUN)) *ident = TRUE;
			if (clear_timed(TMD_CUT)) *ident = TRUE;
			if (clear_timed(TMD_AMNESIA)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURE_FULL2:
		{
			if (hp_player(1200)) *ident = TRUE;
			if (clear_timed(TMD_BLIND)) *ident = TRUE;
			if (clear_timed(TMD_CONFUSED)) *ident = TRUE;
			if (clear_timed(TMD_POISONED)) *ident = TRUE;
			if (clear_timed(TMD_STUN)) *ident = TRUE;
			if (clear_timed(TMD_CUT)) *ident = TRUE;
			if (clear_timed(TMD_AMNESIA)) *ident = TRUE;
			return TRUE;
		}

		case EF_CURE_TEMP:
		{
			if (clear_timed(TMD_BLIND)) *ident = TRUE;
			if (clear_timed(TMD_POISONED)) *ident = TRUE;
			if (clear_timed(TMD_CONFUSED)) *ident = TRUE;
			if (clear_timed(TMD_STUN)) *ident = TRUE;
			if (clear_timed(TMD_CUT)) *ident = TRUE;
			return TRUE;
		}

		case EF_HEAL1:
		{
			if (hp_player(500)) *ident = TRUE;
			if (clear_timed(TMD_CUT)) *ident = TRUE;
			return TRUE;
		}

		case EF_HEAL2:
		{
			if (hp_player(1000)) *ident = TRUE;
			if (clear_timed(TMD_CUT)) *ident = TRUE;
			return TRUE;
		}

		case EF_HEAL3:
		{
			if (hp_player(500)) *ident = TRUE;
			if (clear_timed(TMD_STUN)) *ident = TRUE;
			if (clear_timed(TMD_CUT)) *ident = TRUE;
			return TRUE;
		}


		case EF_GAIN_EXP:
		{
			if (p_ptr->exp < PY_MAX_EXP)
			{
				s32b ee = (p_ptr->exp / 2) + 10;
				if (ee > 100000L) ee = 100000L;
				msg_print("You feel more experienced.");
				gain_exp(ee);
				*ident = TRUE;
			}
			return TRUE;
		}

		case EF_LOSE_EXP:
		{
			if (!p_ptr->hold_life && (p_ptr->exp > 0))
			{
				msg_print("You feel your memories fade.");
				lose_exp(p_ptr->exp / 4);
				*ident = TRUE;
			}
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
				msg_print("Your feel your head clear.");
				p_ptr->redraw |= (PR_MANA);
				p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1);
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

		case EF_LOSE_STR:
		case EF_LOSE_INT:
		case EF_LOSE_WIS:
		case EF_LOSE_DEX:
		case EF_LOSE_CON:
		case EF_LOSE_CHR:
		{
			int stat = effect - EF_LOSE_STR;

			take_hit(damroll(5, 5), "stat drain");
			(void)do_dec_stat(stat);
			*ident = TRUE;

			return TRUE;
		}

		case EF_LOSE_CON2:
		{
			take_hit(damroll(10, 10), "poisonous food");
			(void)do_dec_stat(A_CON);
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
			msg_print("You feel life flow through your body!");
			restore_level();
			(void)clear_timed(TMD_POISONED);
			(void)clear_timed(TMD_BLIND);
			(void)clear_timed(TMD_CONFUSED);
			(void)clear_timed(TMD_IMAGE);
			(void)clear_timed(TMD_STUN);
			(void)clear_timed(TMD_CUT);
			(void)clear_timed(TMD_AMNESIA);

			/* Recalculate max. hitpoints */
			update_stuff();

			hp_player(5000);

			*ident = TRUE;

			/* Now restore all */
		}

		case EF_RESTORE_ALL:
		{
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
			if (inc_timed(TMD_SINFRA, 100 + damroll(4, 25)))
				*ident = TRUE;
			return TRUE;
		}

		case EF_TMD_SINVIS:
		{
			if (inc_timed(TMD_SINVIS, 12 + damroll(2, 6)))
				*ident = TRUE;
			return TRUE;
		}

		case EF_TMD_ESP:
		{
			if (clear_timed(TMD_BLIND)) *ident = TRUE;
			if (inc_timed(TMD_TELEPATHY, 12 + damroll(6, 6)))
				*ident = TRUE;
			return TRUE;
		}


		case EF_ENLIGHTENMENT:
		{
			msg_print("An image of your surroundings forms in your mind...");
			wiz_lite();
			*ident = TRUE;
			return TRUE;
		}


		case EF_ENLIGHTENMENT2:
		{
			msg_print("You begin to feel more enlightened...");
			message_flush();
			wiz_lite();
			(void)do_inc_stat(A_INT);
			(void)do_inc_stat(A_WIS);
			(void)detect_traps();
			(void)detect_doors();
			(void)detect_stairs();
			(void)detect_treasure();
			(void)detect_objects_gold();
			(void)detect_objects_normal();
			identify_pack();
			self_knowledge(TRUE);
			*ident = TRUE;
			return TRUE;
		}

		case EF_SELF_KNOW:
		{
			msg_print("You begin to know yourself a little better...");
			message_flush();
			self_knowledge(TRUE);
			*ident = TRUE;
			return TRUE;
		}


		case EF_HERO:
		{
			if (hp_player(10)) *ident = TRUE;
			if (clear_timed(TMD_AFRAID)) *ident = TRUE;
			if (inc_timed(TMD_HERO, randint(25) + 25)) *ident = TRUE;
			return TRUE;
		}

		case EF_SHERO:
		{
			if (hp_player(30)) *ident = TRUE;
			if (clear_timed(TMD_AFRAID)) *ident = TRUE;
			if (inc_timed(TMD_SHERO, randint(25) + 25)) *ident = TRUE;
			return TRUE;
		}


		case EF_RESIST_FIRE:
		{
			if (inc_timed(TMD_OPP_FIRE, randint(10) + 10))
				*ident = TRUE;
			return TRUE;
		}

		case EF_RESIST_COLD:
		{
			if (inc_timed(TMD_OPP_COLD, randint(10) + 10))
				*ident = TRUE;
			return TRUE;
		}

		case EF_RESIST_ALL:
		{
			if (inc_timed(TMD_OPP_ACID, randint(20) + 20)) *ident = TRUE;
			if (inc_timed(TMD_OPP_ELEC, randint(20) + 20)) *ident = TRUE;
			if (inc_timed(TMD_OPP_FIRE, randint(20) + 20)) *ident = TRUE;
			if (inc_timed(TMD_OPP_COLD, randint(20) + 20)) *ident = TRUE;
			if (inc_timed(TMD_OPP_POIS, randint(20) + 20)) *ident = TRUE;
			return TRUE;
		}


		case EF_DET_GOLD:
		{
			if (detect_treasure()) *ident = TRUE;
			if (detect_objects_gold()) *ident = TRUE;
			return TRUE;
		}

		case EF_DET_OBJ:
		{
			if (detect_objects_normal()) *ident = TRUE;
			return TRUE;
		}

		case EF_DET_TRAP:
		{
			if (detect_traps()) *ident = TRUE;
			return TRUE;
		}

		case EF_DET_DOORSTAIR:
		{
			if (detect_doors()) *ident = TRUE;
			if (detect_stairs()) *ident = TRUE;
			return TRUE;
		}

		case EF_DET_INVIS:
		{
			if (detect_monsters_invis()) *ident = TRUE;
			return TRUE;
		}

		case EF_DET_EVIL:
		{
			if (detect_monsters_evil()) *ident = TRUE;
			return TRUE;
		}

		case EF_DET_ALL:
		{
			if (detect_all()) *ident = TRUE;
			return TRUE;
		}


		case EF_ENCHANT_TOHIT:
		{
			*ident = TRUE;
			if (!enchant_spell(1, 0, 0)) return FALSE;
			return TRUE;
		}

		case EF_ENCHANT_TODAM:
		{
			*ident = TRUE;
			if (!enchant_spell(0, 1, 0)) return FALSE;
			return TRUE;
		}

		case EF_ENCHANT_WEAPON:
		{
			*ident = TRUE;
			if (!enchant_spell(randint(3), randint(3), 0)) return FALSE;
			return TRUE;
		}

		case EF_ENCHANT_ARMOR:
		{
			if (!enchant_spell(0, 0, 1)) return FALSE;
			*ident = TRUE;
			return TRUE;
		}

		case EF_ENCHANT_ARMOR2:
		{
			*ident = TRUE;
			if (!enchant_spell(0, 0, randint(3) + 2)) return FALSE;
			return TRUE;
		}

		case EF_IDENTIFY:
		{
			*ident = TRUE;
			if (!ident_spell()) return FALSE;
			return TRUE;
		}

		case EF_IDENTIFY2:
		{
			*ident = TRUE;
			if (!identify_fully()) return FALSE;
			return TRUE;
		}

		case EF_REMOVE_CURSE:
		{
			if (remove_curse())
			{
				if (!p_ptr->timed[TMD_BLIND])
					msg_print("The air around your body glows blue for a moment...");
				else
					msg_print("You feel as if someone is watching over you.");

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
			if (lite_area(damroll(2, 8), 2)) *ident = TRUE;
			return TRUE;
		}
		
		case EF_SUMMON_MON:
		{
			int i;
			sound(MSG_SUM_MONSTER);

			for (i = 0; i < randint(3); i++)
			{
				if (summon_specific(py, px, p_ptr->depth, 0))
					*ident = TRUE;
			}
			return TRUE;
		}

		case EF_SUMMON_UNDEAD:
		{
			int i;
			sound(MSG_SUM_UNDEAD);

			for (i = 0; i < randint(3); i++)
			{
				if (summon_specific(py, px, p_ptr->depth, SUMMON_UNDEAD))
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
				msg_print("Your hands begin to glow.");
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
			acquirement(py, px, 1, TRUE);
			*ident = TRUE;
			return TRUE;
		}

		case EF_ACQUIRE2:
		{
			acquirement(py, px, randint(2) + 1, TRUE);
			*ident = TRUE;
			return TRUE;
		}

		case EF_ANNOY_MON:
		{
			msg_print("There is a high pitched humming noise.");
			aggravate_monsters(0);
			*ident = TRUE;
			return TRUE;
		}

		case EF_CREATE_TRAP:
		{
			if (trap_creation()) *ident = TRUE;
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
			if (!p_ptr->resist_blind)
			{
				(void)inc_timed(TMD_BLIND, 3 + randint(5));
			}
			if (unlite_area(10, 3)) *ident = TRUE;
			return TRUE;
		}

		case EF_PROTEVIL:
		{
			if (inc_timed(TMD_PROTEVIL, randint(25) + 3 * p_ptr->lev)) *ident = TRUE;
			return TRUE;
		}

		case EF_SATISFY:
		{
			if (set_food(PY_FOOD_MAX - 1)) *ident = TRUE;
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
			if (inc_timed(TMD_BLESSED, randint(12) + 6)) *ident = TRUE;
			return TRUE;
		}

		case EF_BLESSING2:
		{
			if (inc_timed(TMD_BLESSED, randint(24) + 12)) *ident = TRUE;
			return TRUE;
		}

		case EF_BLESSING3:
		{
			if (inc_timed(TMD_BLESSED, randint(48) + 24)) *ident = TRUE;
			return TRUE;
		}

		case EF_RECALL:
		{
			set_recall();
			*ident = TRUE;
			return TRUE;
		}

		case EF_LOSHASTE:
		{
			if (speed_monsters()) *ident = TRUE;
			return TRUE;
		}

		case EF_LOSSLEEP:
		{
			if (sleep_monsters()) *ident = TRUE;
			return TRUE;
		}

		case EF_LOSSLOW:
		{
			if (slow_monsters()) *ident = TRUE;
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
			if (lite_area(damroll(2, 15), 3)) *ident = TRUE;
			return TRUE;
		}

		case EF_CLAIRVOYANCE:
		{
			*ident = TRUE;
			wiz_lite();
			(void)detect_traps();
			(void)detect_doors();
			(void)detect_stairs();
			return TRUE;
		}

		case EF_PROBING:
		{
			probing();
			*ident = TRUE;
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
			confuse_monster(dir, 20);
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
			for (i = 0; i < 8; i++) fire_ball(GF_ELEC, ddd[i], 150, 3);
			return TRUE;
		}

		case EF_RAGE_BLESS_RESIST:
		{
			*ident = TRUE;
			(void)hp_player(30);
			(void)clear_timed(TMD_AFRAID);
			(void)inc_timed(TMD_SHERO, randint(50) + 50);
			(void)inc_timed(TMD_BLESSED, randint(50) + 50);
			(void)inc_timed(TMD_OPP_ACID, randint(50) + 50);
			(void)inc_timed(TMD_OPP_ELEC, randint(50) + 50);
			(void)inc_timed(TMD_OPP_FIRE, randint(50) + 50);
			(void)inc_timed(TMD_OPP_COLD, randint(50) + 50);
			(void)inc_timed(TMD_OPP_POIS, randint(50) + 50);
			return TRUE;
		}

		case EF_SLEEPII:
		{
			*ident = TRUE;
			sleep_monsters_touch();
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
			fire_bolt_or_beam(beam, GF_MISSILE, dir, damroll(3, 4));
			return TRUE;
		}

		case EF_DISPEL_EVIL:
		{
			*ident = TRUE;
			dispel_evil(p_ptr->lev * 5);
			return TRUE;
		}

		case EF_DISPEL_EVIL60:
		{
			if (dispel_evil(60)) *ident = TRUE;
			return TRUE;
		}

		case EF_DISPEL_UNDEAD:
		{
			if (dispel_undead(60)) *ident = TRUE;
			return TRUE;
		}

		case EF_DISPEL_ALL:
		{
			if (dispel_monsters(120)) *ident = TRUE;
			return TRUE;
		}

		case EF_HASTE:
		{
			if (!p_ptr->timed[TMD_FAST])
			{
				if (set_timed(TMD_FAST, damroll(2, 10) + 20)) *ident = TRUE;
			}
			else
			{
				(void)inc_timed(TMD_FAST, 5);
			}

			return TRUE;
		}

		case EF_HASTE1:
		{
			if (!p_ptr->timed[TMD_FAST])
			{
				if (set_timed(TMD_FAST, randint(20) + 20)) *ident = TRUE;
			}
			else
			{
				(void)inc_timed(TMD_FAST, 5);
			}

			return TRUE;
		}

		case EF_HASTE2:
		{
			if (!p_ptr->timed[TMD_FAST])
			{
				if (set_timed(TMD_FAST, randint(75) + 75)) *ident = TRUE;
			}
			else
			{
				(void)inc_timed(TMD_FAST, 5);
			}

			return TRUE;
		}


		case EF_FIRE_BOLT:
		{
			*ident = TRUE;
			fire_bolt(GF_FIRE, dir, damroll(9, 8));
			return TRUE;
		}

		case EF_FIRE_BOLT2:
		{
			fire_bolt_or_beam(beam, GF_FIRE, dir, damroll(12, 8));
			*ident = TRUE;
			return TRUE;
		}

		case EF_FIRE_BOLT3:
		{
			fire_bolt_or_beam(beam, GF_FIRE, dir, damroll(16, 8));
			*ident = TRUE;
			return TRUE;
		}

		case EF_FIRE_BOLT72:
		{
			*ident = TRUE;
			fire_ball(GF_FIRE, dir, 72, 2);
			return TRUE;
		}
		
		case EF_FIRE_BALL:
		{
			fire_ball(GF_FIRE, dir, 144, 2);
			*ident = TRUE;
			return TRUE;
		}

		case EF_FIRE_BALL2:
		{
			*ident = TRUE;
			fire_ball(GF_FIRE, dir, 120, 3);
			return TRUE;
		}

		case EF_COLD_BOLT:
		{
			*ident = TRUE;
			fire_bolt_or_beam(beam, GF_COLD, dir, damroll(6, 8));
			return TRUE;
		}

		case EF_COLD_BOLT2:
		{
			*ident = TRUE;
			fire_bolt(GF_COLD, dir, damroll(12, 8));
			return TRUE;
		}

		case EF_COLD_BALL2:
		{
			*ident = TRUE;
			fire_ball(GF_COLD, dir, 200, 3);
			return TRUE;
		}

		case EF_COLD_BALL50:
		{
			*ident = TRUE;
			fire_ball(GF_COLD, dir, 50, 2);
			return TRUE;
		}

		case EF_COLD_BALL100:
		{
			*ident = TRUE;
			fire_ball(GF_COLD, dir, 100, 2);
			return TRUE;
		}

		case EF_ACID_BOLT:
		{
			*ident = TRUE;
			fire_bolt(GF_ACID, dir, damroll(5, 8));
			return TRUE;
		}

		case EF_ACID_BOLT2:
		{
			fire_bolt_or_beam(beam, GF_ACID, dir, damroll(10, 8));
			*ident = TRUE;
			return TRUE;
		}

		case EF_ACID_BOLT3:
		{
			fire_bolt_or_beam(beam, GF_ACID, dir, damroll(12, 8));
			*ident = TRUE;
			return TRUE;
		}

		case EF_ACID_BALL:
		{
			fire_ball(GF_ACID, dir, 120, 2);
			*ident = TRUE;
			return TRUE;
		}

		case EF_ELEC_BOLT:
		{
			*ident = TRUE;
			fire_bolt_or_beam(beam, GF_ELEC, dir, damroll(6, 6));
			return TRUE;
		}

		case EF_ELEC_BALL:
		{
			fire_ball(GF_ELEC, dir, 64, 2);
			*ident = TRUE;
			return TRUE;
		}

		case EF_ELEC_BALL2:
		{
			*ident = TRUE;
			fire_ball(GF_ELEC, dir, 250, 3);
			return TRUE;
		}


		case EF_ARROW:
		{
			*ident = TRUE;
			fire_bolt(GF_ARROW, dir, 150);
			return TRUE;
		}

		case EF_REM_FEAR_POIS:
		{
			*ident = TRUE;
			(void)clear_timed(TMD_AFRAID);
			(void)clear_timed(TMD_POISONED);
			return TRUE;
		}

		case EF_STINKING_CLOUD:
		{
			*ident = TRUE;
			fire_ball(GF_POIS, dir, 12, 3);
			return TRUE;
		}


		case EF_DRAIN_LIFE1:
		{
			if (drain_life(dir, 90)) *ident = TRUE;
			return TRUE;
		}

		case EF_DRAIN_LIFE2:
		{
			if (drain_life(dir, 120)) *ident = TRUE;
			return TRUE;
		}

		case EF_DRAIN_LIFE3:
		{
			if (drain_life(dir, 150)) *ident = TRUE;
			return TRUE;
		}

		case EF_DRAIN_LIFE4:
		{
			if (drain_life(dir, 250)) *ident = TRUE;
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
			fire_bolt(GF_MANA, dir, damroll(12, 8));
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
			if (confuse_monster(dir, 10)) *ident = TRUE;
			return TRUE;
		}

		case EF_MON_SLEEP:
		{
			if (sleep_monster(dir)) *ident = TRUE;
			return TRUE;
		}

		case EF_MON_CLONE:
		{
			if (fear_monster(dir, 10)) *ident = TRUE;
			return TRUE;
		}

		case EF_MON_SCARE:
		{
			if (clone_monster(dir)) *ident = TRUE;
			return TRUE;
		}

		case EF_LIGHT_LINE:
		{
			msg_print("A line of shimmering blue light appears.");
			lite_line(dir);
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
				msg_print("Light shoots in all directions!");
			for (i = 0; i < 8; i++) lite_line(ddd[i]);
			*ident = TRUE;
			return TRUE;
		}

		case EF_STARLIGHT2:
		{
			int k;
			for (k = 0; k < 8; k++) strong_lite_line(ddd[k]);
			*ident = TRUE;
			return TRUE;
		}

		case EF_BERSERKER:
		{
			if (inc_timed(TMD_SHERO, randint(50) + 50)) *ident = TRUE;
			return TRUE;
		}


		case EF_WONDER:
		{
			if (spell_wonder(dir, randint(100) + p_ptr->lev / 5, beam)) *ident = TRUE;
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
			int which = 2 * rand_int(sizeof(breath_types) / (2 * sizeof(int)));
			if (fire_ball(breath_types[which], dir, breath_types[which + 1], 3))
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
				msg_print("Your feel your head clear.");
				p_ptr->redraw |= (PR_MANA);
				p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1);
			}
			return TRUE;
		}

		case EF_STAFF_HOLY:
		{
			if (dispel_evil(120)) *ident = TRUE;
			if (inc_timed(TMD_PROTEVIL, randint(25) + 3 * p_ptr->lev)) *ident = TRUE;
			if (clear_timed(TMD_POISONED)) *ident = TRUE;
			if (clear_timed(TMD_AFRAID)) *ident = TRUE;
			if (hp_player(50)) *ident = TRUE;
			if (clear_timed(TMD_STUN)) *ident = TRUE;
			if (clear_timed(TMD_CUT)) *ident = TRUE;
			return TRUE;
		}

		case EF_DRINK_GOOD:
		{
			msg_print("You feel less thirsty.");
			*ident = TRUE;
			return TRUE;
		}

		case EF_DRINK_DEATH:
		{
			msg_print("A feeling of Death flows through your body.");
			take_hit(5000, "a potion of Death");
			*ident = TRUE;
			return TRUE;
		}

		case EF_DRINK_RUIN:
		{
			msg_print("Your nerves and muscles feel weak and lifeless!");
			take_hit(damroll(10, 10), "a potion of Ruination");
			(void)dec_stat(A_DEX, 25, TRUE);
			(void)dec_stat(A_WIS, 25, TRUE);
			(void)dec_stat(A_CON, 25, TRUE);
			(void)dec_stat(A_STR, 25, TRUE);
			(void)dec_stat(A_CHR, 25, TRUE);
			(void)dec_stat(A_INT, 25, TRUE);
			*ident = TRUE;
			return TRUE;
		}

		case EF_DRINK_DETONATE:
		{
			msg_print("Massive explosions rupture your body!");
			take_hit(damroll(50, 20), "a potion of Detonation");
			(void)inc_timed(TMD_STUN, 75);
			(void)inc_timed(TMD_CUT, 5000);
			*ident = TRUE;
			return TRUE;
		}

		case EF_DRINK_SALT:
		{
			msg_print("The potion makes you vomit!");
			(void)set_food(PY_FOOD_STARVE - 1);
			(void)clear_timed(TMD_POISONED);
			(void)inc_timed(TMD_PARALYZED, 4);
			*ident = TRUE;
			return TRUE;
		}

		case EF_FOOD_GOOD:
		{
			msg_print("That tastes good.");
			*ident = TRUE;
			return TRUE;
		}

		case EF_FOOD_WAYBREAD:
		{
			msg_print("That tastes good.");
			(void)clear_timed(TMD_POISONED);
			(void)hp_player(damroll(4, 8));
			*ident = TRUE;
			return TRUE;
		}

		case EF_RING_ACID:
		{
			*ident = TRUE;
			fire_ball(GF_ACID, dir, 70, 2);
			inc_timed(TMD_OPP_ACID, randint(20) + 20);
			return TRUE;
		}

		case EF_RING_FLAMES:
		{
			*ident = TRUE;
			fire_ball(GF_FIRE, dir, 80, 2);
			inc_timed(TMD_OPP_FIRE, randint(20) + 20);
			return TRUE;
		}

		case EF_RING_ICE:
		{
			*ident = TRUE;
			fire_ball(GF_COLD, dir, 75, 2);
			inc_timed(TMD_OPP_COLD, randint(20) + 20);
			return TRUE;
		}

		case EF_RING_LIGHTNING:
		{
			*ident = TRUE;
			fire_ball(GF_ELEC, dir, 85, 2);
			inc_timed(TMD_OPP_ELEC, randint(20) + 20);
			return TRUE;
		}

		case EF_DRAGON_BLUE:
		{
			sound(MSG_BR_ELEC);
			msg_print("You breathe lightning.");
			fire_ball(GF_ELEC, dir, 100, 2);
			return TRUE;
		}

		case EF_DRAGON_GREEN:
		{
			sound(MSG_BR_GAS);
			msg_print("You breathe poison gas.");
			fire_ball(GF_POIS, dir, 150, 2);
			return TRUE;
		}

		case EF_DRAGON_RED:
		{
			sound(MSG_BR_FIRE);
			msg_print("You breathe fire.");
			fire_ball(GF_FIRE, dir, 200, 2);
			return TRUE;
		}

		case EF_DRAGON_MULTIHUED:
		{
			static const struct
			{
				int sound;
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

			int chance = randint(5);
			sound(mh[chance].sound);
			msg_format("You breathe %s.", mh[chance].msg);
			fire_ball(mh[chance].typ, dir, 250, 2);
			return TRUE;
		}

		case EF_DRAGON_BRONZE:
		{
			sound(MSG_BR_CONF);
			msg_print("You breathe confusion.");
			fire_ball(GF_CONFUSION, dir, 120, 2);
			return TRUE;
		}

		case EF_DRAGON_GOLD:
		{
			sound(MSG_BR_SOUND);
			msg_print("You breathe sound.");
			fire_ball(GF_SOUND, dir, 130, 2);
			return TRUE;
		}

		case EF_DRAGON_CHAOS:
		{
			int chance = rand_int(2);
			sound(((chance == 1 ? MSG_BR_CHAOS : MSG_BR_DISENCHANT)));
			msg_format("You breathe %s.",
			           ((chance == 1 ? "chaos" : "disenchantment")));
			fire_ball((chance == 1 ? GF_CHAOS : GF_DISENCHANT),
			          dir, 220, 2);
			return TRUE;
		}

		case EF_DRAGON_LAW:
		{
			int chance = rand_int(2);
			sound(((chance == 1 ? MSG_BR_SOUND : MSG_BR_SHARDS)));
			msg_format("You breathe %s.",
			           ((chance == 1 ? "sound" : "shards")));
			fire_ball((chance == 1 ? GF_SOUND : GF_SHARD),
			          dir, 230, 2);
			return TRUE;
		}

		case EF_DRAGON_BALANCE:
		{
			int chance = rand_int(4);
			msg_format("You breathe %s.",
			           ((chance == 1) ? "chaos" :
			            ((chance == 2) ? "disenchantment" :
			             ((chance == 3) ? "sound" : "shards"))));
			fire_ball(((chance == 1) ? GF_CHAOS :
			           ((chance == 2) ? GF_DISENCHANT :
			            ((chance == 3) ? GF_SOUND : GF_SHARD))),
			          dir, 250, 2);
			return TRUE;
		}

		case EF_DRAGON_SHINING:
		{
			int chance = rand_int(2);
			sound(((chance == 0 ? MSG_BR_LIGHT : MSG_BR_DARK)));
			msg_format("You breathe %s.",
			           ((chance == 0 ? "light" : "darkness")));
			fire_ball((chance == 0 ? GF_LITE : GF_DARK), dir, 200, 2);
			return TRUE;
		}

		case EF_DRAGON_POWER:
		{
			sound(MSG_BR_ELEMENTS);
			msg_print("You breathe the elements.");
			fire_ball(GF_MISSILE, dir, 300, 2);
			return TRUE;
		}
	}

	/* Not used */
	msg_print("Effect not handled.");
	return FALSE;
}
