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
#include "attack.h"
#include "cave.h"
#include "monster/monster.h"
#include "spells.h"

/*
 * Determine if a trap affects the player.
 * Always miss 5% of the time, Always hit 5% of the time.
 * Otherwise, match trap power against player armor.
 */
static bool trap_check_hit(int power)
{
	return test_hit(power, p_ptr->state.ac + p_ptr->state.to_a, TRUE);
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

	static const int min_level[] =
	{
		2,		/* Trap door */
		2,		/* Open pit */
		2,		/* Spiked pit */
		2,		/* Poison pit */
		3,		/* Summoning rune */
		1,		/* Teleport rune */
		2,		/* Fire rune */
		2,		/* Acid rune */
		2,		/* Slow rune */
		6,		/* Strength dart */
		6,		/* Dexterity dart */
		6,		/* Constitution dart */
		2,		/* Gas blind */
		1,		/* Gas confuse */
		2,		/* Gas poison */
		2,		/* Gas sleep */
	};

	/* Paranoia */
	if (cave->feat[y][x] != FEAT_INVIS) return;

	/* Pick a trap */
	while (1)
	{
		/* Hack -- pick a trap */
		feat = FEAT_TRAP_HEAD + randint0(16);

		/* Check against minimum depth */
		if (min_level[feat - FEAT_TRAP_HEAD] > p_ptr->depth) continue;

		/* Hack -- no trap doors on quest levels */
		if ((feat == FEAT_TRAP_HEAD + 0x00) && is_quest(p_ptr->depth)) continue;

		/* Hack -- no trap doors on the deepest level */
		if ((feat == FEAT_TRAP_HEAD + 0x00) && (p_ptr->depth >= MAX_DEPTH-1)) continue;

		/* Done */
		break;
	}

	/* Activate the trap */
	cave_set_feat(cave, y, x, feat);
}

/* Places a trap. All traps are untyped until discovered. */
void place_trap(struct cave *c, int y, int x)
{
	assert(cave_in_bounds(c, y, x));
	assert(cave_isempty(c, y, x));

	/* Place an invisible trap */
	cave_set_feat(c, y, x, FEAT_INVIS);
}

/*
 * Handle player hitting a real trap
 */
void hit_trap(int y, int x)
{
	int i, num, dam;

	const char *name = "a trap";


	/* Disturb the player */
	disturb(0, 0);

	/* Analyze XXX XXX XXX */
	switch (cave->feat[y][x])
	{
		case FEAT_TRAP_HEAD + 0x00:
		{
			msg("You fall through a trap door!");
			if (check_state(OF_FEATHER))
			{
				msg("You float gently down to the next level.");
			}
			else
			{
				dam = damroll(2, 8);
				take_hit(dam, name);
			}
			wieldeds_notice_flag(OF_FEATHER);

			/* New depth */
			dungeon_change_level(p_ptr->depth + 1);
			
			break;
		}

		case FEAT_TRAP_HEAD + 0x01:
		{
			msg("You fall into a pit!");
			if (check_state(OF_FEATHER))
			{
				msg("You float gently to the bottom of the pit.");
			}
			else
			{
				dam = damroll(2, 6);
				take_hit(dam, name);
			}
			wieldeds_notice_flag(OF_FEATHER);
			break;
		}

		case FEAT_TRAP_HEAD + 0x02:
		{
			msg("You fall into a spiked pit!");

			if (check_state(OF_FEATHER))
			{
				msg("You float gently to the floor of the pit.");
				msg("You carefully avoid touching the spikes.");
			}
			else
			{
				/* Base damage */
				dam = damroll(2, 6);

				/* Extra spike damage */
				if (one_in_(2))
				{
					msg("You are impaled!");

					dam = dam * 2;
					(void)inc_timed(TMD_CUT, randint1(dam), TRUE);
				}

				/* Take the damage */
				take_hit(dam, name);
			}
			wieldeds_notice_flag(OF_FEATHER);
			break;
		}

		case FEAT_TRAP_HEAD + 0x03:
		{
			msg("You fall into a spiked pit!");

			if (check_state(OF_FEATHER))
			{
				msg("You float gently to the floor of the pit.");
				msg("You carefully avoid touching the spikes.");
			}
			else
			{
				/* Base damage */
				dam = damroll(2, 6);

				/* Extra spike damage */
				if (one_in_(2))
				{
					msg("You are impaled on poisonous spikes!");

					dam = dam * 2;
					(void)inc_timed(TMD_CUT, randint1(dam), TRUE);

					if (check_state(OF_RES_POIS))
					{
						msg("The poison does not affect you!");
					}
					else
					{
						dam = dam * 2;
						(void)inc_timed(TMD_POISONED, randint1(dam), TRUE);
					}

					wieldeds_notice_flag(OF_RES_POIS);
				}

				/* Take the damage */
				take_hit(dam, name);
			}
			wieldeds_notice_flag(OF_FEATHER);

			break;
		}

		case FEAT_TRAP_HEAD + 0x04:
		{
			sound(MSG_SUM_MONSTER);
			msg("You are enveloped in a cloud of smoke!");
			cave->info[y][x] &= ~(CAVE_MARK);
			cave_set_feat(cave, y, x, FEAT_FLOOR);
			num = 2 + randint1(3);
			for (i = 0; i < num; i++)
			{
				(void)summon_specific(y, x, p_ptr->depth, 0, 1);
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x05:
		{
			msg("You hit a teleport trap!");
			teleport_player(100);
			break;
		}

		case FEAT_TRAP_HEAD + 0x06:
		{
			msg("You are enveloped in flames!");
			dam = damroll(4, 6);
			dam = adjust_dam(GF_FIRE, dam, RANDOMISE, check_for_resist(GF_FIRE,
				p_ptr->state.flags, TRUE));
			if (dam) {
				take_hit(dam, "a fire trap");
				inven_damage(GF_FIRE, MIN(dam * 5, 300));
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x07:
		{
			msg("You are splashed with acid!");
			dam = damroll(4, 6);
			dam = adjust_dam(GF_ACID, dam, RANDOMISE, check_for_resist(GF_ACID,
				p_ptr->state.flags, TRUE));
			if (dam) {
				take_hit(dam, "an acid trap");
				inven_damage(GF_ACID, MIN(dam * 5, 300));
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x08:
		{
			if (trap_check_hit(125))
			{
				msg("A small dart hits you!");
				dam = damroll(1, 4);
				take_hit(dam, name);
				(void)inc_timed(TMD_SLOW, randint0(20) + 20, TRUE);
			}
			else
			{
				msg("A small dart barely misses you.");
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x09:
		{
			if (trap_check_hit(125))
			{
				msg("A small dart hits you!");
				dam = damroll(1, 4);
				take_hit(dam, name);
				(void)do_dec_stat(A_STR, FALSE);
			}
			else
			{
				msg("A small dart barely misses you.");
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0A:
		{
			if (trap_check_hit(125))
			{
				msg("A small dart hits you!");
				dam = damroll(1, 4);
				take_hit(dam, name);
				(void)do_dec_stat(A_DEX, FALSE);
			}
			else
			{
				msg("A small dart barely misses you.");
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0B:
		{
			if (trap_check_hit(125))
			{
				msg("A small dart hits you!");
				dam = damroll(1, 4);
				take_hit(dam, name);
				(void)do_dec_stat(A_CON, FALSE);
			}
			else
			{
				msg("A small dart barely misses you.");
			}
			break;
		}

		case FEAT_TRAP_HEAD + 0x0C:
		{
			msg("You are surrounded by a black gas!");
			if (!check_state(OF_RES_BLIND))
				(void)inc_timed(TMD_BLIND, randint0(50) + 25, TRUE);
			wieldeds_notice_flag(OF_RES_BLIND);

			break;
		}

		case FEAT_TRAP_HEAD + 0x0D:
		{
			msg("You are surrounded by a gas of scintillating colors!");
			if (!check_state(OF_RES_CONFU))
				(void)inc_timed(TMD_CONFUSED, randint0(20) + 10, TRUE);
			wieldeds_notice_flag(OF_RES_CONFU);

			break;
		}

		case FEAT_TRAP_HEAD + 0x0E:
		{
			msg("You are surrounded by a pungent green gas!");
			if (!check_state(OF_RES_POIS))
				(void)inc_timed(TMD_POISONED, randint0(20) + 10, TRUE);
			wieldeds_notice_flag(OF_RES_POIS);

			break;
		}

		case FEAT_TRAP_HEAD + 0x0F:
		{
			msg("You are surrounded by a strange white mist!");
			if (!check_state(OF_FREE_ACT))
				(void)inc_timed(TMD_PARALYZED, randint0(10) + 5, TRUE);
			wieldeds_notice_flag(OF_FREE_ACT);

			break;
		}
	}
}
