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
	effect_type index;   /* Effect index */
	bool aim;            /* Whether the effect requires aiming */
	const char *desc;    /* Effect description */
} info_entry;

/*
 * Useful things about effects.
 */
#define MAKE_TABLE
#include "effects.h"
#undef MAKE_TABLE


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
 * Do an effect, given an object.
 */
bool do_effect(object_type *o_ptr, bool *ident, int dir)
{
	effect_type effect = k_info[o_ptr->k_idx].effect;
	int py = p_ptr->py;
	int px = p_ptr->px;

	if (o_ptr->name1)
		effect = a_info[o_ptr->name1].effect;

	switch (effect)
	{
		case EF_POISON:
		{
			if (!(p_ptr->resist_pois || p_ptr->timed[TMD_OPP_POIS]))
			{
				if (inc_timed(TMD_POISONED, rand_int(10) + 10))
					*ident = TRUE;
			}

			return TRUE;
		}

		case EF_BLIND:
		{
			if (!p_ptr->resist_blind && inc_timed(TMD_BLIND, rand_int(200) + 200))
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
			if (!p_ptr->resist_confu && inc_timed(TMD_CONFUSED, rand_int(10) + 10))
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
			if (!p_ptr->free_act && inc_timed(TMD_PARALYZED, rand_int(10) + 10))
				*ident = TRUE;

			return TRUE;
		}

		case EF_LOSE_STR:
		{
			take_hit(damroll(6, 6), "poisonous food");
			(void)do_dec_stat(A_STR);
			*ident = TRUE;

			return TRUE;
		}

		case EF_LOSE_STR2:
		{
			take_hit(damroll(10, 10), "poisonous food");
			(void)do_dec_stat(A_STR);
			*ident = TRUE;

			return TRUE;
		}

		case EF_LOSE_CON:
		{
			take_hit(damroll(6, 6), "poisonous food");
			(void)do_dec_stat(A_CON);
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

		case EF_LOSE_INT:
		{
			take_hit(damroll(8, 8), "poisonous food");
			(void)do_dec_stat(A_INT);
			*ident = TRUE;

			return TRUE;
		}

		case EF_LOSE_WIS:
		{
			take_hit(damroll(8, 8), "poisonous food");
			(void)do_dec_stat(A_WIS);
			*ident = TRUE;

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

		case EF_CW_SERIOUS:
		{
			if (hp_player(damroll(4, 8))) *ident = TRUE;
			if (set_timed(TMD_CUT, (p_ptr->timed[TMD_CUT] / 2) - 50)) *ident = TRUE;
			return TRUE;
		}

		case EF_RESTORE_STR:
		{
			if (do_res_stat(A_STR)) *ident = TRUE;
			return TRUE;
		}

		case EF_RESTORE_CON:
		{
			if (do_res_stat(A_CON)) *ident = TRUE;
			return TRUE;
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

		case EF_LOSKILL:
		{
			(void)mass_banishment();
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
		case EF_DISPEL_UNDEAD:
		{
			if (dispel_undead(60)) *ident = TRUE;
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

		case EF_DISP_EVIL:
		{
			*ident = TRUE;
			dispel_evil(p_ptr->lev * 5);
			return TRUE;
		}

		case EF_HASTE2:
		{
			*ident = TRUE;
			if (!p_ptr->timed[TMD_FAST])
				(void)set_timed(TMD_FAST, randint(75) + 75);
			else
				(void)inc_timed(TMD_FAST, 5);
			return TRUE;
		}

		case EF_FIRE3:
		{
			*ident = TRUE;
			fire_ball(GF_FIRE, dir, 120, 3);
			return TRUE;
		}

		case EF_FROST5:
		{
			*ident = TRUE;
			fire_ball(GF_COLD, dir, 200, 3);
			return TRUE;
		}

		case EF_ELEC2:
		{
			*ident = TRUE;
			fire_ball(GF_ELEC, dir, 250, 3);
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

		case EF_HEAL2:
		{
			*ident = TRUE;
			(void)hp_player(1000);
			(void)clear_timed(TMD_CUT);
			return TRUE;
		}

		case EF_DETECT_ALL:
		{
				*ident = TRUE;
			detect_all();
			return TRUE;
		}

		case EF_HEAL1:
		{
			*ident = TRUE;
			(void)hp_player(500);
			(void)clear_timed(TMD_CUT);
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
			fire_bolt(GF_MISSILE, dir, damroll(2, 6));
			return TRUE;
		}

		case EF_FIRE1:
		{
			*ident = TRUE;
			fire_bolt(GF_FIRE, dir, damroll(9, 8));
			return TRUE;
		}

		case EF_FROST1:
		{
			*ident = TRUE;
			fire_bolt(GF_COLD, dir, damroll(6, 8));
			return TRUE;
		}

		case EF_LIGHTNING_BOLT:
		{
			*ident = TRUE;
			fire_bolt(GF_ELEC, dir, damroll(4, 8));
			return TRUE;
		}

		case EF_ACID1:
		{
			*ident = TRUE;
			fire_bolt(GF_ACID, dir, damroll(5, 8));
			return TRUE;
		}

		case EF_ARROW:
		{
			*ident = TRUE;
			fire_bolt(GF_ARROW, dir, 150);
			return TRUE;
		}

		case EF_HASTE1:
		{
			*ident = TRUE;
			if (!p_ptr->timed[TMD_FAST])
				(void)set_timed(TMD_FAST, randint(20) + 20);
			else
				(void)inc_timed(TMD_FAST, 5);
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

		case EF_FROST2:
		{
			*ident = TRUE;
			fire_ball(GF_COLD, dir, 48, 2);
			return TRUE;
		}

		case EF_FROST4:
		{
			*ident = TRUE;
			fire_bolt(GF_COLD, dir, damroll(12, 8));
			return TRUE;
		}

		case EF_FROST3:
		{
			*ident = TRUE;
			fire_ball(GF_COLD, dir, 100, 2);
			return TRUE;
		}

		case EF_FIRE2:
		{
			*ident = TRUE;
			fire_ball(GF_FIRE, dir, 72, 2);
			return TRUE;
		}

		case EF_DRAIN_LIFE2:
		{
			*ident = TRUE;
			drain_life(dir, 120);
			return TRUE;
		}

		case EF_STONE_TO_MUD:
		{
			*ident = TRUE;
			wall_to_mud(dir);
			return TRUE;
		}

		case EF_TELE_AWAY:
		{
			*ident = TRUE;
			teleport_monster(dir);
			return TRUE;
		}

		case EF_CONFUSE2:
		{
			*ident = TRUE;
			confuse_monster(dir, 20);
			return TRUE;
		}

		case EF_PROBE:
		{
			*ident = TRUE;
			probing();
			return TRUE;
		}

		case EF_DRAIN_LIFE1:
		{
			*ident = TRUE;
			drain_life(dir, 90);
			return TRUE;
		}

		case EF_FIREBRAND:
		{
			*ident = TRUE;
			if (!brand_bolts()) return FALSE;
			return TRUE;
		}
 
		case EF_STARLIGHT:
		{
			int k;
			for (k = 0; k < 8; k++) strong_lite_line(ddd[k]);
			*ident = TRUE;
			return TRUE;
		}

		case EF_MANA_BOLT:
		{
			fire_bolt(GF_MANA, dir, damroll(12, 8));
			*ident = TRUE;
			return TRUE;
		}

		case EF_BERSERKER:
		{
			if (inc_timed(TMD_SHERO, randint(50) + 50)) *ident = TRUE;
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
	}

	/* Not used */
	msg_print("Effect not handled.");
	return FALSE;
}
