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

#if 0

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

#endif



/*
 * Do an effect, given an object.
 */
bool do_effect(object_type *o_ptr, bool *ident, int dir)
{
	effect_type effect = k_info[o_ptr->k_idx].effect;

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
	}

	/* Not used */
	msg_print("Effect not handled.");
	return FALSE;
}

