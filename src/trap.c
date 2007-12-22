/*
 * File: trap.c
 * Purpose: Trap triggering, selection, and placement
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


/*
 * Determine if a trap affects the player.
 * Always miss 5% of the time, Always hit 5% of the time.
 * Otherwise, match trap power against player armor.
 */
static bool check_hit(int power)
{
	return test_hit(power, p_ptr->ac + p_ptr->to_a, TRUE);
}


/*
 * Hack -- instantiate a trap
 *
 * XXX XXX XXX This routine should be redone to reflect trap "level".
 * That is, it does not make sense to have spiked pits at 50 feet.
 * Actually, it is not this routine, but the "trap instantiation"
 * code, which should also check for "trap doors" on quest levels.
 */
void pick_trap(int y, int x)
{
	int feat;

	/* Paranoia */
	if (cave_feat[y][x] != FEAT_INVIS) return;

	/* Pick a trap */
	while (1)
	{
		/* Hack -- pick a trap */
		feat = FEAT_TRAP_HEAD + rand_int(16);

		/* Hack -- no trap doors on quest levels */
		if ((feat == FEAT_TRAP_HEAD + 0x00) && is_quest(p_ptr->depth)) continue;

		/* Hack -- no trap doors on the deepest level */
		if ((feat == FEAT_TRAP_HEAD + 0x00) && (p_ptr->depth >= MAX_DEPTH-1)) continue;

		/* Done */
		break;
	}

	/* Activate the trap */
	cave_set_feat(y, x, feat);
}



/*
 * Places a random trap at the given location.
 *
 * The location must be a legal, naked, floor grid.
 *
 * Note that all traps start out as "invisible" and "untyped", and then
 * when they are "discovered" (by detecting them or setting them off),
 * the trap is "instantiated" as a visible, "typed", trap.
 */
void place_trap(int y, int x)
{
	/* Paranoia */
	if (!in_bounds(y, x)) return;

	/* Require empty, clean, floor grid */
	if (!cave_naked_bold(y, x)) return;

	/* Place an invisible trap */
	cave_set_feat(y, x, FEAT_INVIS);
}




/*
 * Handle player hitting a real trap
 */
void hit_trap(int y, int x)
{
	int i, num, dam;

	cptr name = "a trap";


	/* Disturb the player */
	disturb(0, 0);

	/* Analyze XXX XXX XXX */
	switch (cave_feat[y][x])
	{
		case FEAT_TRAP_HEAD + 0x00:
		{
			msg_print("You fall through a trap door!");
			if (p_ptr->ffall)
			{
				msg_print("You float gently down to the next level.");
			}
			else
			{
				dam = damroll(2, 8);
				take_hit(dam, name);
			}

			/* New depth */
			p_ptr->depth++;

			/* Leaving */
			p_ptr->leaving = TRUE;

			break;
		}

		case FEAT_TRAP_HEAD + 0x01:
		{
			msg_print("You fall into a pit!");
			if (p_ptr->ffall)
			{
				msg_print("You float gently to the bottom of the pit.");
			}
			else
			{
				dam = damroll(2, 6);
				take_hit(dam, name);
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x02:
		{
			msg_print("You fall into a spiked pit!");

			if (p_ptr->ffall)
			{
				msg_print("You float gently to the floor of the pit.");
				msg_print("You carefully avoid touching the spikes.");
			}

			else
			{
				/* Base damage */
				dam = damroll(2, 6);

				/* Extra spike damage */
				if (one_in_(2))
				{
					msg_print("You are impaled!");

					dam = dam * 2;
					(void)inc_timed(TMD_CUT, randint(dam));
				}

				/* Take the damage */
				take_hit(dam, name);
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x03:
		{
			msg_print("You fall into a spiked pit!");

			if (p_ptr->ffall)
			{
				msg_print("You float gently to the floor of the pit.");
				msg_print("You carefully avoid touching the spikes.");
			}

			else
			{
				/* Base damage */
				dam = damroll(2, 6);

				/* Extra spike damage */
				if (one_in_(2))
				{
					msg_print("You are impaled on poisonous spikes!");

					dam = dam * 2;
					(void)inc_timed(TMD_CUT, randint(dam));

					if (p_ptr->resist_pois || p_ptr->timed[TMD_OPP_POIS])
					{
						msg_print("The poison does not affect you!");
					}
					else
					{
						dam = dam * 2;
						(void)inc_timed(TMD_POISONED, randint(dam));
					}
				}

				/* Take the damage */
				take_hit(dam, name);
			}

			break;
		}

		case FEAT_TRAP_HEAD + 0x04:
		{
			sound(MSG_SUM_MONSTER);
			msg_print("You are enveloped in a cloud of smoke!");
			cave_info[y][x] &= ~(CAVE_MARK);
			cave_set_feat(y, x, FEAT_FLOOR);
			num = 2 + randint(3);
			for (i = 0; i < num; i++)
			{
				(void)summon_specific(y, x, p_ptr->depth, 0);
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x05:
		{
			msg_print("You hit a teleport trap!");
			teleport_player(100);
			break;
		}

		case FEAT_TRAP_HEAD + 0x06:
		{
			msg_print("You are enveloped in flames!");
			dam = damroll(4, 6);
			fire_dam(dam, "a fire trap");
			break;
		}

		case FEAT_TRAP_HEAD + 0x07:
		{
			msg_print("You are splashed with acid!");
			dam = damroll(4, 6);
			acid_dam(dam, "an acid trap");
			break;
		}

		case FEAT_TRAP_HEAD + 0x08:
		{
			if (check_hit(125))
			{
				msg_print("A small dart hits you!");
				dam = damroll(1, 4);
				take_hit(dam, name);
				(void)inc_timed(TMD_SLOW, rand_int(20) + 20);
			}
			else
			{
				msg_print("A small dart barely misses you.");
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x09:
		{
			if (check_hit(125))
			{
				msg_print("A small dart hits you!");
				dam = damroll(1, 4);
				take_hit(dam, name);
				(void)do_dec_stat(A_STR, FALSE);
			}
			else
			{
				msg_print("A small dart barely misses you.");
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0A:
		{
			if (check_hit(125))
			{
				msg_print("A small dart hits you!");
				dam = damroll(1, 4);
				take_hit(dam, name);
				(void)do_dec_stat(A_DEX, FALSE);
			}
			else
			{
				msg_print("A small dart barely misses you.");
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0B:
		{
			if (check_hit(125))
			{
				msg_print("A small dart hits you!");
				dam = damroll(1, 4);
				take_hit(dam, name);
				(void)do_dec_stat(A_CON, FALSE);
			}
			else
			{
				msg_print("A small dart barely misses you.");
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0C:
		{
			msg_print("You are surrounded by a black gas!");
			if (!p_ptr->resist_blind)
			{
				(void)inc_timed(TMD_BLIND, rand_int(50) + 25);
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0D:
		{
			msg_print("You are surrounded by a gas of scintillating colors!");
			if (!p_ptr->resist_confu)
			{
				(void)inc_timed(TMD_CONFUSED, rand_int(20) + 10);
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0E:
		{
			msg_print("You are surrounded by a pungent green gas!");
			if (!p_ptr->resist_pois && !p_ptr->timed[TMD_OPP_POIS])
			{
				(void)inc_timed(TMD_POISONED, rand_int(20) + 10);
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0F:
		{
			msg_print("You are surrounded by a strange white mist!");
			if (!p_ptr->free_act)
			{
				(void)inc_timed(TMD_PARALYZED, rand_int(10) + 5);
			}
			break;
		}
	}
}
