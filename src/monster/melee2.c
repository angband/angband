/*
 * File: melee2.c
 * Purpose: Monster AI routines
 *
 * Copyright (c) 1997 Ben Harrison, David Reeve Sward, Keldon Jones.
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
#include "bitflag.h"
#include "object/tvalsval.h"
#include "monster/constants.h"

/*
 * Determine if a bolt will arrive, checking that no monsters are in the way
 */
#define clean_shot(Y1, X1, Y2, X2) \
	projectable(Y1, X1, Y2, X2, PROJECT_STOP)


/*
 * And now for Intelligent monster attacks (including spells).
 *
 * Give monsters more intelligent attack/spell selection based on
 * observations of previous attacks on the player, and/or by allowing
 * the monster to "cheat" and know the player status.
 *
 * Maintain an idea of the player status, and use that information
 * to occasionally eliminate "ineffective" spell attacks.  We could
 * also eliminate ineffective normal attacks, but there is no reason
 * for the monster to do this, since he gains no benefit.
 * Note that MINDLESS monsters are not allowed to use this code.
 * And non-INTELLIGENT monsters only use it partially effectively.
 *
 * Actually learn what the player resists, and use that information
 * to remove attacks or spells before using them.  This will require
 * much less space, if I am not mistaken.  Thus, each monster gets a
 * set of 32 bit flags, "smart", build from the various "SM_*" flags.
 *
 * This has the added advantage that attacks and spells are related.
 * The "smart_learn" option means that the monster "learns" the flags
 * that should be set, and "smart_cheat" means that he "knows" them.
 * So "smart_cheat" means that the "smart" field is always up to date,
 * while "smart_learn" means that the "smart" field is slowly learned.
 * Both of them have the same effect on the "choose spell" routine.
 */




/*
 * Internal probability routine
 */
static bool int_outof(const monster_race *r_ptr, int prob)
{
	/* Non-Smart monsters are half as "smart" */
	if (!(r_ptr->flags[1] & (RF1_SMART))) prob = prob / 2;

	/* Roll the dice */
	return (randint0(100) < prob);
}



/*
 * Remove the "bad" spells from a spell list
 */
static void remove_bad_spells(int m_idx, u32b* const f)
{
	monster_type *m_ptr = &mon_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	u32b f2[RACE_FLAG_SPELL_STRICT_UB];
	
	u32b smart = 0L;


	/* Too stupid to know anything */
	if (r_ptr->flags[1] & (RF1_STUPID)) return;


	/* Must be cheating or learning */
	if (!OPT(adult_ai_cheat) && !OPT(adult_ai_learn)) return;

	/* take working copy of spell flags */
	race_flags_assign_spell(f2, f);

	/* Update acquired knowledge */
	if (OPT(adult_ai_learn))
	{
		/* Hack -- Occasionally forget player status */
		if (m_ptr->smart && (randint0(100) < 1)) m_ptr->smart = 0L;

		/* Use the memorized flags */
		smart = m_ptr->smart;
	}


	/* Cheat if requested */
	if (OPT(adult_ai_cheat))
	{
		/* Know weirdness */
		if (p_ptr->state.free_act) smart |= (SM_IMM_FREE);
		if (!p_ptr->msp) smart |= (SM_IMM_MANA);

		/* Know immunities */
		if (p_ptr->state.immune_acid) smart |= (SM_IMM_ACID);
		if (p_ptr->state.immune_elec) smart |= (SM_IMM_ELEC);
		if (p_ptr->state.immune_fire) smart |= (SM_IMM_FIRE);
		if (p_ptr->state.immune_cold) smart |= (SM_IMM_COLD);

		/* Know oppositions */
		if (p_ptr->timed[TMD_OPP_ACID]) smart |= (SM_OPP_ACID);
		if (p_ptr->timed[TMD_OPP_ELEC]) smart |= (SM_OPP_ELEC);
		if (p_ptr->timed[TMD_OPP_FIRE]) smart |= (SM_OPP_FIRE);
		if (p_ptr->timed[TMD_OPP_COLD]) smart |= (SM_OPP_COLD);
		if (p_ptr->timed[TMD_OPP_POIS]) smart |= (SM_OPP_POIS);

		/* Know resistances */
		if (p_ptr->state.resist_acid) smart |= (SM_RES_ACID);
		if (p_ptr->state.resist_elec) smart |= (SM_RES_ELEC);
		if (p_ptr->state.resist_fire) smart |= (SM_RES_FIRE);
		if (p_ptr->state.resist_cold) smart |= (SM_RES_COLD);
		if (p_ptr->state.resist_pois) smart |= (SM_RES_POIS);
		if (p_ptr->state.resist_fear) smart |= (SM_RES_FEAR);
		if (p_ptr->state.resist_lite) smart |= (SM_RES_LITE);
		if (p_ptr->state.resist_dark) smart |= (SM_RES_DARK);
		if (p_ptr->state.resist_blind) smart |= (SM_RES_BLIND);
		if (p_ptr->state.resist_confu) smart |= (SM_RES_CONFU);
		if (p_ptr->state.resist_sound) smart |= (SM_RES_SOUND);
		if (p_ptr->state.resist_shard) smart |= (SM_RES_SHARD);
		if (p_ptr->state.resist_nexus) smart |= (SM_RES_NEXUS);
		if (p_ptr->state.resist_nethr) smart |= (SM_RES_NETHR);
		if (p_ptr->state.resist_chaos) smart |= (SM_RES_CHAOS);
		if (p_ptr->state.resist_disen) smart |= (SM_RES_DISEN);
	}


	/* Nothing known */
	if (!smart) return;


	if (smart & (SM_IMM_ACID))
	{
		if (int_outof(r_ptr, 100)) f2[0] &= ~(RSF0_BR_ACID);
		if (int_outof(r_ptr, 100)) f2[1] &= ~(RSF1_BA_ACID);
		if (int_outof(r_ptr, 100)) f2[1] &= ~(RSF1_BO_ACID);
	}
	else if ((smart & (SM_OPP_ACID)) && (smart & (SM_RES_ACID)))
	{
		if (int_outof(r_ptr, 80)) f2[0] &= ~(RSF0_BR_ACID);
		if (int_outof(r_ptr, 80)) f2[1] &= ~(RSF1_BA_ACID);
		if (int_outof(r_ptr, 80)) f2[1] &= ~(RSF1_BO_ACID);
	}
	else if ((smart & (SM_OPP_ACID)) || (smart & (SM_RES_ACID)))
	{
		if (int_outof(r_ptr, 30)) f2[0] &= ~(RSF0_BR_ACID);
		if (int_outof(r_ptr, 30)) f2[1] &= ~(RSF1_BA_ACID);
		if (int_outof(r_ptr, 30)) f2[1] &= ~(RSF1_BO_ACID);
	}


	if (smart & (SM_IMM_ELEC))
	{
		if (int_outof(r_ptr, 100)) f2[0] &= ~(RSF0_BR_ELEC);
		if (int_outof(r_ptr, 100)) f2[1] &= ~(RSF1_BA_ELEC);
		if (int_outof(r_ptr, 100)) f2[1] &= ~(RSF1_BO_ELEC);
	}
	else if ((smart & (SM_OPP_ELEC)) && (smart & (SM_RES_ELEC)))
	{
		if (int_outof(r_ptr, 80)) f2[0] &= ~(RSF0_BR_ELEC);
		if (int_outof(r_ptr, 80)) f2[1] &= ~(RSF1_BA_ELEC);
		if (int_outof(r_ptr, 80)) f2[1] &= ~(RSF1_BO_ELEC);
	}
	else if ((smart & (SM_OPP_ELEC)) || (smart & (SM_RES_ELEC)))
	{
		if (int_outof(r_ptr, 30)) f2[0] &= ~(RSF0_BR_ELEC);
		if (int_outof(r_ptr, 30)) f2[1] &= ~(RSF1_BA_ELEC);
		if (int_outof(r_ptr, 30)) f2[1] &= ~(RSF1_BO_ELEC);
	}


	if (smart & (SM_IMM_FIRE))
	{
		if (int_outof(r_ptr, 100)) f2[0] &= ~(RSF0_BR_FIRE);
		if (int_outof(r_ptr, 100)) f2[1] &= ~(RSF1_BA_FIRE);
		if (int_outof(r_ptr, 100)) f2[1] &= ~(RSF1_BO_FIRE);
	}
	else if ((smart & (SM_OPP_FIRE)) && (smart & (SM_RES_FIRE)))
	{
		if (int_outof(r_ptr, 80)) f2[0] &= ~(RSF0_BR_FIRE);
		if (int_outof(r_ptr, 80)) f2[1] &= ~(RSF1_BA_FIRE);
		if (int_outof(r_ptr, 80)) f2[1] &= ~(RSF1_BO_FIRE);
	}
	else if ((smart & (SM_OPP_FIRE)) || (smart & (SM_RES_FIRE)))
	{
		if (int_outof(r_ptr, 30)) f2[0] &= ~(RSF0_BR_FIRE);
		if (int_outof(r_ptr, 30)) f2[1] &= ~(RSF1_BA_FIRE);
		if (int_outof(r_ptr, 30)) f2[1] &= ~(RSF1_BO_FIRE);
	}


	if (smart & (SM_IMM_COLD))
	{
		if (int_outof(r_ptr, 100)) f2[0] &= ~(RSF0_BR_COLD);
		if (int_outof(r_ptr, 100)) f2[1] &= ~(RSF1_BA_COLD);
		if (int_outof(r_ptr, 100)) f2[1] &= ~(RSF1_BO_COLD);
		if (int_outof(r_ptr, 100)) f2[1] &= ~(RSF1_BO_ICEE);
	}
	else if ((smart & (SM_OPP_COLD)) && (smart & (SM_RES_COLD)))
	{
		if (int_outof(r_ptr, 80)) f2[0] &= ~(RSF0_BR_COLD);
		if (int_outof(r_ptr, 80)) f2[1] &= ~(RSF1_BA_COLD);
		if (int_outof(r_ptr, 80)) f2[1] &= ~(RSF1_BO_COLD);
		if (int_outof(r_ptr, 80)) f2[1] &= ~(RSF1_BO_ICEE);
	}
	else if ((smart & (SM_OPP_COLD)) || (smart & (SM_RES_COLD)))
	{
		if (int_outof(r_ptr, 30)) f2[0] &= ~(RSF0_BR_COLD);
		if (int_outof(r_ptr, 30)) f2[1] &= ~(RSF1_BA_COLD);
		if (int_outof(r_ptr, 30)) f2[1] &= ~(RSF1_BO_COLD);
		if (int_outof(r_ptr, 30)) f2[1] &= ~(RSF1_BO_ICEE);
	}


	if ((smart & (SM_OPP_POIS)) && (smart & (SM_RES_POIS)))
	{
		if (int_outof(r_ptr, 80)) f2[0] &= ~(RSF0_BR_POIS);
		if (int_outof(r_ptr, 80)) f2[1] &= ~(RSF1_BA_POIS);
	}
	else if ((smart & (SM_OPP_POIS)) || (smart & (SM_RES_POIS)))
	{
		if (int_outof(r_ptr, 30)) f2[0] &= ~(RSF0_BR_POIS);
		if (int_outof(r_ptr, 30)) f2[1] &= ~(RSF1_BA_POIS);
	}


	if (smart & (SM_RES_FEAR))
	{
		if (int_outof(r_ptr, 100)) f2[1] &= ~(RSF1_SCARE);
	}

	if (smart & (SM_RES_LITE))
	{
		if (int_outof(r_ptr, 50)) f2[0] &= ~(RSF0_BR_LITE);
	}

	if (smart & (SM_RES_DARK))
	{
		if (int_outof(r_ptr, 50)) f2[0] &= ~(RSF0_BR_DARK);
		if (int_outof(r_ptr, 50)) f2[1] &= ~(RSF1_BA_DARK);
	}

	if (smart & (SM_RES_BLIND))
	{
		if (int_outof(r_ptr, 100)) f2[1] &= ~(RSF1_BLIND);
	}

	if (smart & (SM_RES_CONFU))
	{
		if (int_outof(r_ptr, 100)) f2[1] &= ~(RSF1_CONF);
		if (int_outof(r_ptr, 50)) f2[0] &= ~(RSF0_BR_CONF);
	}

	if (smart & (SM_RES_SOUND))
	{
		if (int_outof(r_ptr, 50)) f2[0] &= ~(RSF0_BR_SOUN);
	}

	if (smart & (SM_RES_SHARD))
	{
		if (int_outof(r_ptr, 50)) f2[0] &= ~(RSF0_BR_SHAR);
	}

	if (smart & (SM_RES_NEXUS))
	{
		if (int_outof(r_ptr, 50)) f2[0] &= ~(RSF0_BR_NEXU);
		if (int_outof(r_ptr, 50)) f2[2] &= ~(RSF2_TELE_LEVEL);
	}

	if (smart & (SM_RES_NETHR))
	{
		if (int_outof(r_ptr, 50)) f2[0] &= ~(RSF0_BR_NETH);
		if (int_outof(r_ptr, 50)) f2[1] &= ~(RSF1_BA_NETH);
		if (int_outof(r_ptr, 50)) f2[1] &= ~(RSF1_BO_NETH);
	}

	if (smart & (SM_RES_CHAOS))
	{
		if (int_outof(r_ptr, 50)) f2[0] &= ~(RSF0_BR_CHAO);
	}

	if (smart & (SM_RES_DISEN))
	{
		if (int_outof(r_ptr, 100)) f2[0] &= ~(RSF0_BR_DISE);
	}


	if (smart & (SM_IMM_FREE))
	{
		if (int_outof(r_ptr, 100)) f2[1] &= ~(RSF1_HOLD);
		if (int_outof(r_ptr, 100)) f2[1] &= ~(RSF1_SLOW);
	}

	if (smart & (SM_IMM_MANA))
	{
		if (int_outof(r_ptr, 100)) f2[1] &= ~(RSF1_DRAIN_MANA);
	}


	/* XXX XXX XXX No spells left? */
	/* if (!f4 && !f5 && !f6) ... */

	/* use working copy of spell flags */
	race_flags_assign_spell(f, f2);
}


/*
 * Determine if there is a space near the selected spot in which
 * a summoned creature can appear
 */
static bool summon_possible(int y1, int x1)
{
	int y, x;

	/* Start at the location, and check 2 grids in each dir */
	for (y = y1 - 2; y <= y1 + 2; y++)
	{
		for (x = x1 - 2; x <= x1 + 2; x++)
		{
			/* Ignore illegal locations */
			if (!in_bounds(y, x)) continue;

			/* Only check a circular area */
			if (distance(y1, x1, y, x) > 2) continue;

			/* Hack: no summon on glyph of warding */
			if (cave_feat[y][x] == FEAT_GLYPH) continue;

			/* Require empty floor grid in line of sight */
			if (cave_empty_bold(y, x) && los(y1, x1, y, x))
			{
				return (TRUE);
			}
		}
	}

	return FALSE;
}



/*
 * Cast a bolt at the player
 * Stop if we hit a monster
 * Affect monsters and the player
 */
static void bolt(int m_idx, int typ, int dam_hp)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int flg = PROJECT_STOP | PROJECT_KILL;

	/* Target the player with a bolt attack */
	(void)project(m_idx, 0, py, px, dam_hp, typ, flg);
}


/*
 * Cast a breath (or ball) attack at the player
 * Pass over any monsters that may be in the way
 * Affect grids, objects, monsters, and the player
 */
static void breath(int m_idx, int typ, int dam_hp)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int rad;

	int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

	monster_type *m_ptr = &mon_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	/* Determine the radius of the blast */
	rad = (r_ptr->flags[1] & (RF1_POWERFUL)) ? 3 : 2;

	/* Target the player with a ball attack */
	(void)project(m_idx, rad, py, px, dam_hp, typ, flg);
}


/*
 * Offsets for the spell indices
 */
#define BASE2_LOG_HACK_FRAGMENT(A,B) ((((A)>>(B)) & 1)*(B))

/** Unfortunately, this macro only works for an isolated bitflag 
 * (exactly one bit set, all others reset).  It also only works for at most 
 * 32 bits. 
 */
#define BASE2_LOG_HACK(A)	\
	(BASE2_LOG_HACK_FRAGMENT(A,31)+BASE2_LOG_HACK_FRAGMENT(A,30)+ \
	 BASE2_LOG_HACK_FRAGMENT(A,29)+BASE2_LOG_HACK_FRAGMENT(A,28)+ \
	 BASE2_LOG_HACK_FRAGMENT(A,27)+BASE2_LOG_HACK_FRAGMENT(A,26)+ \
	 BASE2_LOG_HACK_FRAGMENT(A,25)+BASE2_LOG_HACK_FRAGMENT(A,24)+ \
	 BASE2_LOG_HACK_FRAGMENT(A,23)+BASE2_LOG_HACK_FRAGMENT(A,22)+ \
	 BASE2_LOG_HACK_FRAGMENT(A,21)+BASE2_LOG_HACK_FRAGMENT(A,20)+ \
	 BASE2_LOG_HACK_FRAGMENT(A,19)+BASE2_LOG_HACK_FRAGMENT(A,18)+ \
	 BASE2_LOG_HACK_FRAGMENT(A,17)+BASE2_LOG_HACK_FRAGMENT(A,16)+ \
	 BASE2_LOG_HACK_FRAGMENT(A,15)+BASE2_LOG_HACK_FRAGMENT(A,14)+ \
	 BASE2_LOG_HACK_FRAGMENT(A,13)+BASE2_LOG_HACK_FRAGMENT(A,12)+ \
	 BASE2_LOG_HACK_FRAGMENT(A,11)+BASE2_LOG_HACK_FRAGMENT(A,10)+ \
	 BASE2_LOG_HACK_FRAGMENT(A,9)+BASE2_LOG_HACK_FRAGMENT(A,8)+ \
	 BASE2_LOG_HACK_FRAGMENT(A,7)+BASE2_LOG_HACK_FRAGMENT(A,6)+ \
	 BASE2_LOG_HACK_FRAGMENT(A,5)+BASE2_LOG_HACK_FRAGMENT(A,4)+ \
	 BASE2_LOG_HACK_FRAGMENT(A,3)+BASE2_LOG_HACK_FRAGMENT(A,2)+ \
	 BASE2_LOG_HACK_FRAGMENT(A,1))

#define SPELL_ORIGIN 1
#define MIN_NONINNATE_SPELL (SPELL_ORIGIN+32)

/** converts spell flag into an index value for the case statement */
#define SPELL(A,B) SPELL_ORIGIN+(A)*32+BASE2_LOG_HACK(B)

/*
 * Have a monster choose a spell to cast.
 *
 * Note that the monster's spell list has already had "useless" spells
 * (bolts that won't hit the player, summons without room, etc.) removed.
 * Perhaps that should be done by this function.
 *
 * Stupid monsters will just pick a spell randomly.  Smart monsters
 * will choose more "intelligently".
 *
 * This function could be an efficiency bottleneck.
 */
static int choose_attack_spell(int m_idx, u32b f[RACE_FLAG_SPELL_STRICT_UB])
{
	monster_type *m_ptr = &mon_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	u32b f3_mask = 0L;
	u32b f4_mask = 0L;
	u32b f5_mask = 0L;

	int num = 0;
	byte spells[96];

	int i, py = p_ptr->py, px = p_ptr->px;

	bool has_escape, has_attack, has_summon, has_tactic;
	bool has_annoy, has_haste, has_heal;


	/* Smart monsters restrict their spell choices. */
	if (OPT(adult_ai_smart) && !(r_ptr->flags[1] & (RF1_STUPID)))
	{
		/* What have we got? */
		has_escape = ((f[0] & (RSF0_ESCAPE_MASK)) ||
		              (f[1] & (RSF1_ESCAPE_MASK)) ||
		              (f[2] & (RSF2_ESCAPE_MASK)));
		has_attack = ((f[0] & (RSF0_ATTACK_MASK)) ||
		              (f[1] & (RSF1_ATTACK_MASK)) ||
		              (f[2] & (RSF2_ATTACK_MASK)));
		has_summon = ((f[0] & (RSF0_SUMMON_MASK)) ||
		              (f[1] & (RSF1_SUMMON_MASK)) ||
		              (f[2] & (RSF2_SUMMON_MASK)));
		has_tactic = ((f[0] & (RSF0_TACTIC_MASK)) ||
		              (f[1] & (RSF1_TACTIC_MASK)) ||
		              (f[2] & (RSF2_TACTIC_MASK)));
		has_annoy = ((f[0] & (RSF0_ANNOY_MASK)) ||
		             (f[1] & (RSF1_ANNOY_MASK)) ||
		             (f[2] & (RSF2_ANNOY_MASK)));
		has_haste = ((f[0] & (RSF0_HASTE_MASK)) ||
		             (f[1] & (RSF1_HASTE_MASK)) ||
		             (f[2] & (RSF2_HASTE_MASK)));
		has_heal = ((f[0] & (RSF0_HEAL_MASK)) ||
		            (f[1] & (RSF1_HEAL_MASK)) ||
		            (f[2] & (RSF2_HEAL_MASK)));

		/*** Try to pick an appropriate spell type ***/

		/* Hurt badly or afraid, attempt to flee */
		if (has_escape && ((m_ptr->hp < m_ptr->maxhp / 4) || m_ptr->monfear))
		{
			/* Choose escape spell */
			f3_mask = (RSF0_ESCAPE_MASK);
			f4_mask = (RSF1_ESCAPE_MASK);
			f5_mask = (RSF2_ESCAPE_MASK);
		}

		/* Still hurt badly, couldn't flee, attempt to heal */
		else if (has_heal && m_ptr->hp < m_ptr->maxhp / 4)
		{
			/* Choose heal spell */
			f3_mask = (RSF0_HEAL_MASK);
			f4_mask = (RSF1_HEAL_MASK);
			f5_mask = (RSF2_HEAL_MASK);
		}

		/* Player is close and we have attack spells, blink away */
		else if (has_tactic && (distance(py, px, m_ptr->fy, m_ptr->fx) < 4) &&
		         has_attack && (randint0(100) < 75))
		{
			/* Choose tactical spell */
			f3_mask = (RSF0_TACTIC_MASK);
			f4_mask = (RSF1_TACTIC_MASK);
			f5_mask = (RSF2_TACTIC_MASK);
		}

		/* We're hurt (not badly), try to heal */
		else if (has_heal && (m_ptr->hp < m_ptr->maxhp * 3 / 4) &&
		         (randint0(100) < 60))
		{
			/* Choose heal spell */
			f3_mask = (RSF0_HEAL_MASK);
			f4_mask = (RSF1_HEAL_MASK);
			f5_mask = (RSF2_HEAL_MASK);
		}

		/* Summon if possible (sometimes) */
		else if (has_summon && (randint0(100) < 50))
		{
			/* Choose summon spell */
			f3_mask = (RSF0_SUMMON_MASK);
			f4_mask = (RSF1_SUMMON_MASK);
			f5_mask = (RSF2_SUMMON_MASK);
		}

		/* Attack spell (most of the time) */
		else if (has_attack && (randint0(100) < 85))
		{
			/* Choose attack spell */
			f3_mask = (RSF0_ATTACK_MASK);
			f4_mask = (RSF1_ATTACK_MASK);
			f5_mask = (RSF2_ATTACK_MASK);
		}

		/* Try another tactical spell (sometimes) */
		else if (has_tactic && (randint0(100) < 50))
		{
			/* Choose tactic spell */
			f3_mask = (RSF0_TACTIC_MASK);
			f4_mask = (RSF1_TACTIC_MASK);
			f5_mask = (RSF2_TACTIC_MASK);
		}

		/* Haste self if we aren't already somewhat hasted (rarely) */
		else if (has_haste && (randint0(100) < (20 + r_ptr->speed - m_ptr->mspeed)))
		{
			/* Choose haste spell */
			f3_mask = (RSF0_HASTE_MASK);
			f4_mask = (RSF1_HASTE_MASK);
			f5_mask = (RSF2_HASTE_MASK);
		}

		/* Annoy player (most of the time) */
		else if (has_annoy && (randint0(100) < 85))
		{
			/* Choose annoyance spell */
			f3_mask = (RSF0_ANNOY_MASK);
			f4_mask = (RSF1_ANNOY_MASK);
			f5_mask = (RSF2_ANNOY_MASK);
		}

		/* Else choose no spell (The masks default to this.) */

		/* Keep only the interesting spells */
		f[0] &= f3_mask;
		f[1] &= f4_mask;
		f[2] &= f5_mask;

		/* Anything left? */
		if (!(f[0] || f[1] || f[2])) return (0);
	}

	/* Extract all spells: "innate", "normal", "bizarre" */
	for (i = 0; i < 32*RACE_FLAG_SPELL_STRICT_UB; i++)
	{
		if (TEST_FLAG(f, i)) spells[num++] = i + SPELL_ORIGIN;
	}

	/* Paranoia */
	if (num == 0) return 0;

	/* Pick at random */
	return (spells[randint0(num)]);
}


/*
 * Creatures can cast spells, shoot missiles, and breathe.
 *
 * Returns "TRUE" if a spell (or whatever) was (successfully) cast.
 *
 * XXX XXX XXX This function could use some work, but remember to
 * keep it as optimized as possible, while retaining generic code.
 *
 * Verify the various "blind-ness" checks in the code.
 *
 * XXX XXX XXX Note that several effects should really not be "seen"
 * if the player is blind.
 *
 * Perhaps monsters should breathe at locations *near* the player,
 * since this would allow them to inflict "partial" damage.
 *
 * Perhaps smart monsters should decline to use "bolt" spells if
 * there is a monster in the way, unless they wish to kill it.
 *
 * It will not be possible to "correctly" handle the case in which a
 * monster attempts to attack a location which is thought to contain
 * the player, but which in fact is nowhere near the player, since this
 * might induce all sorts of messages about the attack itself, and about
 * the effects of the attack, which the player might or might not be in
 * a position to observe.  Thus, for simplicity, it is probably best to
 * only allow "faulty" attacks by a monster if one of the important grids
 * (probably the initial or final grid) is in fact in view of the player.
 * It may be necessary to actually prevent spell attacks except when the
 * monster actually has line of sight to the player.  Note that a monster
 * could be left in a bizarre situation after the player ducked behind a
 * pillar and then teleported away, for example.
 *
 * Note that certain spell attacks do not use the "project()" function
 * but "simulate" it via the "direct" variable, which is always at least
 * as restrictive as the "project()" function.  This is necessary to
 * prevent "blindness" attacks and such from bending around walls.
 *
 * Note that this function attempts to optimize the use of spells for the
 * cases in which the monster has no spells, or has spells but cannot use
 * them, or has spells but they will have no "useful" effect.  Note that
 * this function has been an efficiency bottleneck in the past.
 *
 * Note the special "MFLAG_NICE" flag, which prevents a monster from using
 * any spell attacks until the player has had a single chance to move.
 */
bool make_attack_spell(int m_idx)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int k, chance, thrown_spell, rlev;

	int failrate;

	u32b f[RACE_FLAG_SPELL_STRICT_UB];

	monster_type *m_ptr = &mon_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	monster_lore *l_ptr = &l_list[m_ptr->r_idx];

	char m_name[80];
	char m_poss[80];

	char ddesc[80];

	/* Summon count */
	int count = 0;


	/* Extract the blind-ness */
	bool blind = (p_ptr->timed[TMD_BLIND] ? TRUE : FALSE);

	/* Extract the "see-able-ness" */
	bool seen = (!blind && m_ptr->ml);


	/* Assume "normal" target */
	bool normal = TRUE;

	/* Assume "projectable" */
	bool direct = TRUE;


	/* Cannot cast spells when confused */
	if (m_ptr->confused) return (FALSE);

	/* Cannot cast spells when nice */
	if (m_ptr->mflag & (MFLAG_NICE)) return (FALSE);

	/* Hack -- Extract the spell probability */
	chance = (r_ptr->freq_innate + r_ptr->freq_spell) / 2;

	/* Not allowed to cast spells */
	if (!chance) return (FALSE);


	/* Only do spells occasionally */
	if (randint0(100) >= chance) return (FALSE);


	/* Hack -- require projectable player */
	if (normal)
	{
		/* Check range */
		if (m_ptr->cdis > MAX_RANGE) return (FALSE);

		/* Check path */
		if (!projectable(m_ptr->fy, m_ptr->fx, py, px, PROJECT_NONE))
			return (FALSE);
	}


	/* Extract the monster level */
	rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);


	/* Extract the racial spell flags */
	race_flags_assign_spell(f, r_ptr->spell_flags);


	/* Hack -- allow "desperate" spells */
	if ((r_ptr->flags[1] & (RF1_SMART)) &&
	    (m_ptr->hp < m_ptr->maxhp / 10) &&
	    (randint0(100) < 50))
	{
		/* Require intelligent spells */
		f[0] &= (RSF0_INT_MASK);
		f[1] &= (RSF1_INT_MASK);
		f[2] &= (RSF2_INT_MASK);

		/* No spells left */
		if (!f[0] && !f[1] && !f[2]) return (FALSE);
	}


	/* Remove the "ineffective" spells */
	remove_bad_spells(m_idx, f);

	/* No spells left */
	if (!f[0] && !f[1] && !f[2]) return (FALSE);

	/* Check whether summons and bolts are worth it. */
	if (!(r_ptr->flags[1] & RF1_STUPID))
	{
		/* Check for a clean bolt shot */
		if ((f[0] & (RSF0_BOLT_MASK) ||
			 f[1] & (RSF1_BOLT_MASK) ||
			 f[2] & (RSF2_BOLT_MASK)) &&
			!clean_shot(m_ptr->fy, m_ptr->fx, py, px))
		{
			/* Remove spells that will only hurt friends */
			f[0] &= ~(RSF0_BOLT_MASK);
			f[1] &= ~(RSF1_BOLT_MASK);
			f[2] &= ~(RSF2_BOLT_MASK);
		}

		/* Check for a possible summon */
		if (!(summon_possible(m_ptr->fy, m_ptr->fx)))
		{
			/* Remove summoning spells */
			f[0] &= ~(RSF0_SUMMON_MASK);
			f[1] &= ~(RSF1_SUMMON_MASK);
			f[2] &= ~(RSF2_SUMMON_MASK);
		}

		/* No spells left */
		if (!f[0] && !f[1] && !f[2]) return (FALSE);
	}

	/* Handle "leaving" */
	if (p_ptr->leaving) return (FALSE);


	/* Get the monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), m_ptr, 0x00);

	/* Get the monster possessive ("his"/"her"/"its") */
	monster_desc(m_poss, sizeof(m_poss), m_ptr, MDESC_PRO2 | MDESC_POSS);

	/* Hack -- Get the "died from" name */
	monster_desc(ddesc, sizeof(ddesc), m_ptr, MDESC_SHOW | MDESC_IND2);


	/* Choose a spell to cast */
	thrown_spell = choose_attack_spell(m_idx, f);

	/* Abort if no spell was chosen */
	if (!thrown_spell) return (FALSE);

	/* Calculate spell failure rate */
	failrate = 25 - (rlev + 3) / 4;

	/* Hack -- Stupid monsters will never fail (for jellies and such) */
	if (!OPT(adult_ai_smart) || r_ptr->flags[1] & (RF1_STUPID)) failrate = 0;

	/* Check for spell failure (innate attacks never fail) */
	if ((thrown_spell >= MIN_NONINNATE_SPELL) && (randint0(100) < failrate))
	{
		/* Message */
		msg_format("%^s tries to cast a spell, but fails.", m_name);

		return (TRUE);
	}

	/* Cast the spell. */
	switch (thrown_spell)
	{
		case SPELL(0,RSF0_SHRIEK):
		{
			if (!direct) break;
			disturb(1, 0);
			sound(MSG_SHRIEK);
			msg_format("%^s makes a high pitched shriek.", m_name);
			aggravate_monsters(m_idx);
			break;
		}

		case SPELL(0,RSF0_XXX2):
		{
			break;
		}

		case SPELL(0,RSF0_XXX3):
		{
			break;
		}

		case SPELL(0,RSF0_XXX4):
		{
			break;
		}

		case SPELL(0,RSF0_ARROW_1):
		{
			bool hits = check_hit(ARROW1_HIT, rlev);
			int dam = ARROW1_DMG(rlev, RANDOMISE);
			disturb(1, 0);

			if (blind) msg_format("%^s makes a strange noise.", m_name);
			else if (hits) msg_format("%^s fires an arrow!", m_name);
			else msg_format("%^s fires an arrow, but misses.", m_name);

			/* dam -= (dam * ((p_ptr->state.ac < 150) ? p_ptr->state.ac : 150) / 250); */
			if (hits) bolt(m_idx, GF_ARROW, dam);

			break;
		}

		case SPELL(0,RSF0_ARROW_2):
		{
			bool hits = check_hit(ARROW2_HIT, rlev);
			int dam = ARROW2_DMG(rlev, RANDOMISE);
			disturb(1, 0);

			if (blind) msg_format("%^s makes a strange noise.", m_name);
			else if (hits) msg_format("%^s fires an arrow!", m_name);
			else msg_format("%^s fires an arrow, but misses.", m_name);

			/* dam -= (dam * ((p_ptr->state.ac < 150) ? p_ptr->state.ac : 150) / 250); */
			if (hits) bolt(m_idx, GF_ARROW, dam);

			break;
		}

		case SPELL(0,RSF0_ARROW_3):
		{
			bool hits = check_hit(ARROW3_HIT, rlev);
			int dam = ARROW3_DMG(rlev, RANDOMISE);
			disturb(1, 0);

			if (blind) msg_format("%^s makes a strange noise.", m_name);
			else if (hits) msg_format("%^s fires a missile!", m_name);
			else msg_format("%^s fires a missile, but misses.", m_name);

			/* dam -= (dam * ((p_ptr->state.ac < 150) ? p_ptr->state.ac : 150) / 250); */
			if (hits) bolt(m_idx, GF_ARROW, dam);

			break;
		}

		case SPELL(0,RSF0_ARROW_4):
		{
			bool hits = check_hit(ARROW4_HIT, rlev);
			int dam = ARROW4_DMG(rlev, RANDOMISE);
			disturb(1, 0);

			if (blind) msg_format("%^s makes a strange noise.", m_name);
			else if (hits) msg_format("%^s fires a missile!", m_name);
			else msg_format("%^s fires a missile, but misses.", m_name);

			/* dam -= (dam * ((p_ptr->state.ac < 150) ? p_ptr->state.ac : 150) / 250); */
			if (hits) bolt(m_idx, GF_ARROW, dam);

			break;
		}

		case SPELL(0,RSF0_BR_ACID):
		{
			disturb(1, 0);
			sound(MSG_BR_ACID);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes acid.", m_name);
			breath(m_idx, GF_ACID,
			       ((m_ptr->hp / BR_ACID_DIVISOR) > BR_ACID_MAX ? BR_ACID_MAX : (m_ptr->hp / BR_ACID_DIVISOR)));
			update_smart_learn(m_idx, DRS_RES_ACID);
			break;
		}

		case SPELL(0,RSF0_BR_ELEC):
		{
			disturb(1, 0);
			sound(MSG_BR_ELEC);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes lightning.", m_name);
			breath(m_idx, GF_ELEC,
			       ((m_ptr->hp / BR_ELEC_DIVISOR) > BR_ELEC_MAX ? BR_ELEC_MAX : (m_ptr->hp / BR_ELEC_DIVISOR)));
			update_smart_learn(m_idx, DRS_RES_ELEC);
			break;
		}

		case SPELL(0,RSF0_BR_FIRE):
		{
			disturb(1, 0);
			sound(MSG_BR_FIRE);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes fire.", m_name);
			breath(m_idx, GF_FIRE,
			       ((m_ptr->hp / BR_FIRE_DIVISOR) > BR_FIRE_MAX ? BR_FIRE_MAX : (m_ptr->hp / BR_FIRE_DIVISOR)));
			update_smart_learn(m_idx, DRS_RES_FIRE);
			break;
		}

		case SPELL(0,RSF0_BR_COLD):
		{
			disturb(1, 0);
			sound(MSG_BR_FROST);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes frost.", m_name);
			breath(m_idx, GF_COLD,
			       ((m_ptr->hp / BR_COLD_DIVISOR) > BR_COLD_MAX ? BR_COLD_MAX : (m_ptr->hp / BR_COLD_DIVISOR)));
			update_smart_learn(m_idx, DRS_RES_COLD);
			break;
		}

		case SPELL(0,RSF0_BR_POIS):
		{
			disturb(1, 0);
			sound(MSG_BR_GAS);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes gas.", m_name);
			breath(m_idx, GF_POIS,
			       ((m_ptr->hp / BR_POIS_DIVISOR) > BR_POIS_MAX ? BR_POIS_MAX : (m_ptr->hp / BR_POIS_DIVISOR)));
			update_smart_learn(m_idx, DRS_RES_POIS);
			break;
		}

		case SPELL(0,RSF0_BR_NETH):
		{
			disturb(1, 0);
			sound(MSG_BR_NETHER);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes nether.", m_name);
			breath(m_idx, GF_NETHER,
			       ((m_ptr->hp / BR_NETH_DIVISOR) > BR_NETH_MAX ? BR_NETH_MAX : (m_ptr->hp / BR_NETH_DIVISOR)));
			update_smart_learn(m_idx, DRS_RES_NETHR);
			break;
		}

		case SPELL(0,RSF0_BR_LITE):
		{
			disturb(1, 0);
			sound(MSG_BR_LIGHT);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes light.", m_name);
			breath(m_idx, GF_LITE,
			       ((m_ptr->hp / BR_LITE_DIVISOR) > BR_LITE_MAX ? BR_LITE_MAX : (m_ptr->hp / BR_LITE_DIVISOR)));
			update_smart_learn(m_idx, DRS_RES_LITE);
			break;
		}

		case SPELL(0,RSF0_BR_DARK):
		{
			disturb(1, 0);
			sound(MSG_BR_DARK);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes darkness.", m_name);
			breath(m_idx, GF_DARK,
			       ((m_ptr->hp / BR_DARK_DIVISOR) > BR_DARK_MAX ? BR_DARK_MAX : (m_ptr->hp / BR_DARK_DIVISOR)));
			update_smart_learn(m_idx, DRS_RES_DARK);
			break;
		}

		case SPELL(0,RSF0_BR_CONF):
		{
			disturb(1, 0);
			sound(MSG_BR_CONF);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes confusion.", m_name);
			breath(m_idx, GF_CONFUSION,
			       ((m_ptr->hp / BR_CONF_DIVISOR) > BR_CONF_MAX ? BR_CONF_MAX : (m_ptr->hp / BR_CONF_DIVISOR)));
			update_smart_learn(m_idx, DRS_RES_CONFU);
			break;
		}

		case SPELL(0,RSF0_BR_SOUN):
		{
			disturb(1, 0);
			sound(MSG_BR_SOUND);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes sound.", m_name);
			breath(m_idx, GF_SOUND,
			       ((m_ptr->hp / BR_SOUN_DIVISOR) > BR_SOUN_MAX ? BR_SOUN_MAX : (m_ptr->hp / BR_SOUN_DIVISOR)));
			update_smart_learn(m_idx, DRS_RES_SOUND);
			break;
		}

		case SPELL(0,RSF0_BR_CHAO):
		{
			disturb(1, 0);
			sound(MSG_BR_CHAOS);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes chaos.", m_name);
			breath(m_idx, GF_CHAOS,
			       ((m_ptr->hp / BR_CHAO_DIVISOR) > BR_CHAO_MAX ? BR_CHAO_MAX : (m_ptr->hp / BR_CHAO_DIVISOR)));
			update_smart_learn(m_idx, DRS_RES_CHAOS);
			break;
		}

		case SPELL(0,RSF0_BR_DISE):
		{
			disturb(1, 0);
			sound(MSG_BR_DISENCHANT);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes disenchantment.", m_name);
			breath(m_idx, GF_DISENCHANT,
			       ((m_ptr->hp / BR_DISE_DIVISOR) > BR_DISE_MAX ? BR_DISE_MAX : (m_ptr->hp / BR_DISE_DIVISOR)));
			update_smart_learn(m_idx, DRS_RES_DISEN);
			break;
		}

		case SPELL(0,RSF0_BR_NEXU):
		{
			disturb(1, 0);
			sound(MSG_BR_NEXUS);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes nexus.", m_name);
			breath(m_idx, GF_NEXUS,
			       ((m_ptr->hp / BR_NEXU_DIVISOR) > BR_NEXU_MAX ? BR_NEXU_MAX : (m_ptr->hp / BR_NEXU_DIVISOR)));
			update_smart_learn(m_idx, DRS_RES_NEXUS);
			break;
		}

		case SPELL(0,RSF0_BR_TIME):
		{
			disturb(1, 0);
			sound(MSG_BR_TIME);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes time.", m_name);
			breath(m_idx, GF_TIME,
			       ((m_ptr->hp / BR_TIME_DIVISOR) > BR_TIME_MAX ? BR_TIME_MAX : (m_ptr->hp / BR_TIME_DIVISOR)));
			break;
		}

		case SPELL(0,RSF0_BR_INER):
		{
			disturb(1, 0);
			sound(MSG_BR_INERTIA);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes inertia.", m_name);
			breath(m_idx, GF_INERTIA,
			       ((m_ptr->hp / BR_INER_DIVISOR) > BR_INER_MAX ? BR_INER_MAX : (m_ptr->hp / BR_INER_DIVISOR)));
			break;
		}

		case SPELL(0,RSF0_BR_GRAV):
		{
			disturb(1, 0);
			sound(MSG_BR_GRAVITY);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes gravity.", m_name);
			breath(m_idx, GF_GRAVITY,
			       ((m_ptr->hp / BR_GRAV_DIVISOR) > BR_GRAV_MAX ? BR_GRAV_MAX : (m_ptr->hp / BR_GRAV_DIVISOR)));
			break;
		}

		case SPELL(0,RSF0_BR_SHAR):
		{
			disturb(1, 0);
			sound(MSG_BR_SHARDS);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes shards.", m_name);
			breath(m_idx, GF_SHARD,
			       ((m_ptr->hp / BR_SHAR_DIVISOR) > BR_SHAR_MAX ? BR_SHAR_MAX : (m_ptr->hp / BR_SHAR_DIVISOR)));
			update_smart_learn(m_idx, DRS_RES_SHARD);
			break;
		}

		case SPELL(0,RSF0_BR_PLAS):
		{
			disturb(1, 0);
			sound(MSG_BR_PLASMA);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes plasma.", m_name);
			breath(m_idx, GF_PLASMA,
			       ((m_ptr->hp / BR_PLAS_DIVISOR) > BR_PLAS_MAX ? BR_PLAS_MAX : (m_ptr->hp / BR_PLAS_DIVISOR)));
			break;
		}

		case SPELL(0,RSF0_BR_WALL):
		{
			disturb(1, 0);
			sound(MSG_BR_FORCE);
			if (blind) msg_format("%^s breathes.", m_name);
			else msg_format("%^s breathes force.", m_name);
			breath(m_idx, GF_FORCE,
			       ((m_ptr->hp / BR_FORC_DIVISOR) > BR_FORC_MAX ? BR_FORC_MAX : (m_ptr->hp / BR_FORC_DIVISOR)));
			break;
		}

		case SPELL(0,RSF0_BR_MANA):
		{
			/* XXX XXX XXX */
			break;
		}

		case SPELL(0,RSF0_XXX5):
		{
			break;
		}

		case SPELL(0,RSF0_XXX6):
		{
			break;
		}

		case SPELL(0,RSF0_XXX7):
		{
			break;
		}

		case SPELL(0,RSF0_BOULDER):
		{
			bool hits = check_hit(BOULDER_HIT, rlev);
			int dam = BOULDER_DMG(rlev, RANDOMISE);
			disturb(1, 0);

			if (blind) msg_format("You hear something grunt with exertion.", m_name);
			else if (hits) msg_format("%^s hurls a boulder at you!", m_name);
			else msg_format("%^s hurls a boulder at you, but misses.", m_name);

			/* dam -= (dam * ((p_ptr->state.ac < 150) ? p_ptr->state.ac : 150) / 250); */
			if (hits) bolt(m_idx, GF_ARROW, dam);

			break;
		}


		case SPELL(1,RSF1_BA_ACID):
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s casts an acid ball.", m_name);
			breath(m_idx, GF_ACID,
			       BA_ACID_DMG(rlev, RANDOMISE));
			update_smart_learn(m_idx, DRS_RES_ACID);
			break;
		}

		case SPELL(1,RSF1_BA_ELEC):
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s casts a lightning ball.", m_name);
			breath(m_idx, GF_ELEC,
			       BA_ELEC_DMG(rlev, RANDOMISE));
			update_smart_learn(m_idx, DRS_RES_ELEC);
			break;
		}

		case SPELL(1,RSF1_BA_FIRE):
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s casts a fire ball.", m_name);
			breath(m_idx, GF_FIRE,
			       BA_FIRE_DMG(rlev, RANDOMISE));
			update_smart_learn(m_idx, DRS_RES_FIRE);
			break;
		}

		case SPELL(1,RSF1_BA_COLD):
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s casts a frost ball.", m_name);
			breath(m_idx, GF_COLD,
			       BA_COLD_DMG(rlev, RANDOMISE));
			update_smart_learn(m_idx, DRS_RES_COLD);
			break;
		}

		case SPELL(1,RSF1_BA_POIS):
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s casts a stinking cloud.", m_name);
			breath(m_idx, GF_POIS,
			       BA_POIS_DMG(rlev, RANDOMISE));
			update_smart_learn(m_idx, DRS_RES_POIS);
			break;
		}

		case SPELL(1,RSF1_BA_NETH):
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s casts a nether ball.", m_name);
			breath(m_idx, GF_NETHER,
			       BA_NETH_DMG(rlev, RANDOMISE));
			update_smart_learn(m_idx, DRS_RES_NETHR);
			break;
		}

		case SPELL(1,RSF1_BA_WATE):
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s gestures fluidly.", m_name);
			msg_print("You are engulfed in a whirlpool.");
			breath(m_idx, GF_WATER,
			       BA_WATE_DMG(rlev, RANDOMISE));
			break;
		}

		case SPELL(1,RSF1_BA_MANA):
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles powerfully.", m_name);
			else msg_format("%^s invokes a mana storm.", m_name);
			breath(m_idx, GF_MANA,
			       BA_MANA_DMG(rlev, RANDOMISE));
			break;
		}

		case SPELL(1,RSF1_BA_DARK):
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles powerfully.", m_name);
			else msg_format("%^s invokes a darkness storm.", m_name);
			breath(m_idx, GF_DARK,
			       BA_DARK_DMG(rlev, RANDOMISE));
			update_smart_learn(m_idx, DRS_RES_DARK);
			break;
		}

		case SPELL(1,RSF1_DRAIN_MANA):
		{
			if (!direct) break;
			if (p_ptr->csp)
			{
				int r1;

				/* Disturb if legal */
				disturb(1, 0);

				/* Basic message */
				msg_format("%^s draws psychic energy from you!", m_name);

				/* Attack power */
				r1 = (randint1(rlev) / 2) + 1;

				/* Full drain */
				if (r1 >= p_ptr->csp)
				{
					r1 = p_ptr->csp;
					p_ptr->csp = 0;
					p_ptr->csp_frac = 0;
				}

				/* Partial drain */
				else
				{
					p_ptr->csp -= r1;
				}

				/* Redraw mana */
				p_ptr->redraw |= (PR_MANA);

				/* Heal the monster */
				if (m_ptr->hp < m_ptr->maxhp)
				{
					/* Heal */
					m_ptr->hp += (6 * r1);
					if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

					/* Redraw (later) if needed */
					if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

					/* Special message */
					if (seen)
					{
						msg_format("%^s appears healthier.", m_name);
					}
				}
			}
			update_smart_learn(m_idx, DRS_MANA);
			break;
		}

		case SPELL(1,RSF1_MIND_BLAST):
		{
			if (!direct) break;
			disturb(1, 0);
			if (!seen)
			{
				msg_print("You feel something focusing on your mind.");
			}
			else
			{
				msg_format("%^s gazes deep into your eyes.", m_name);
			}

			if (randint0(100) < p_ptr->state.skills[SKILL_SAVE])
			{
				msg_print("You resist the effects!");
			}
			else
			{
				msg_print("Your mind is blasted by psionic energy.");
				if (!p_ptr->state.resist_confu)
					(void)inc_timed(TMD_CONFUSED, randint0(4) + 4, TRUE);
				else
					wieldeds_notice_flag(1, TR1_RES_CONFU);

				take_hit(MIND_BLAST_DMG(rlev, RANDOMISE), ddesc);
			}
			break;
		}

		case SPELL(1,RSF1_BRAIN_SMASH):
		{
			if (!direct) break;
			disturb(1, 0);
			if (!seen)
				msg_print("You feel something focusing on your mind.");
			else
				msg_format("%^s looks deep into your eyes.", m_name);

			if (randint0(100) < p_ptr->state.skills[SKILL_SAVE])
			{
				msg_print("You resist the effects!");
			}
			else
			{
				msg_print("Your mind is blasted by psionic energy.");
				take_hit(BRAIN_SMASH_DMG(rlev, RANDOMISE), ddesc);
				if (!p_ptr->state.resist_blind)
					(void)inc_timed(TMD_BLIND, 8 + randint0(8), TRUE);
				else
					wieldeds_notice_flag(1, TR1_RES_BLIND);

				if (!p_ptr->state.resist_confu)
					(void)inc_timed(TMD_CONFUSED, randint0(4) + 4, TRUE);
				else
					wieldeds_notice_flag(1, TR1_RES_CONFU);

				if (!p_ptr->state.free_act)
					(void)inc_timed(TMD_PARALYZED, randint0(4) + 4, TRUE);
				else
					wieldeds_notice_flag(2, TR2_FREE_ACT);

				(void)inc_timed(TMD_SLOW, randint0(4) + 4, TRUE);
			}
			break;
		}

		case SPELL(1,RSF1_CAUSE_1):
		{
			if (!direct) break;
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s points at you and curses.", m_name);
			if (randint0(100) < p_ptr->state.skills[SKILL_SAVE])
			{
				msg_print("You resist the effects!");
			}
			else
			{
				take_hit(CAUSE1_DMG(rlev, RANDOMISE), ddesc);
			}
			break;
		}

		case SPELL(1,RSF1_CAUSE_2):
		{
			if (!direct) break;
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s points at you and curses horribly.", m_name);
			if (randint0(100) < p_ptr->state.skills[SKILL_SAVE])
			{
				msg_print("You resist the effects!");
			}
			else
			{
				take_hit(CAUSE2_DMG(rlev, RANDOMISE), ddesc);
			}
			break;
		}

		case SPELL(1,RSF1_CAUSE_3):
		{
			if (!direct) break;
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles loudly.", m_name);
			else msg_format("%^s points at you, incanting terribly!", m_name);
			if (randint0(100) < p_ptr->state.skills[SKILL_SAVE])
			{
				msg_print("You resist the effects!");
			}
			else
			{
				take_hit(CAUSE3_DMG(rlev, RANDOMISE), ddesc);
			}
			break;
		}

		case SPELL(1,RSF1_CAUSE_4):
		{
			if (!direct) break;
			disturb(1, 0);
			if (blind) msg_format("%^s screams the word 'DIE!'", m_name);
			else msg_format("%^s points at you, screaming the word DIE!", m_name);
			if (randint0(100) < p_ptr->state.skills[SKILL_SAVE])
			{
				msg_print("You resist the effects!");
			}
			else
			{
				take_hit(CAUSE4_DMG(rlev, RANDOMISE), ddesc);
				(void)inc_timed(TMD_CUT, CAUSE4_CUT, TRUE);
			}
			break;
		}

		case SPELL(1,RSF1_BO_ACID):
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s casts a acid bolt.", m_name);
			bolt(m_idx, GF_ACID,
			     BO_ACID_DMG(rlev, RANDOMISE));
			update_smart_learn(m_idx, DRS_RES_ACID);
			break;
		}

		case SPELL(1,RSF1_BO_ELEC):
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s casts a lightning bolt.", m_name);
			bolt(m_idx, GF_ELEC,
			     BO_ELEC_DMG(rlev, RANDOMISE));
			update_smart_learn(m_idx, DRS_RES_ELEC);
			break;
		}

		case SPELL(1,RSF1_BO_FIRE):
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s casts a fire bolt.", m_name);
			bolt(m_idx, GF_FIRE,
			     BO_FIRE_DMG(rlev, RANDOMISE));
			update_smart_learn(m_idx, DRS_RES_FIRE);
			break;
		}

		case SPELL(1,RSF1_BO_COLD):
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s casts a frost bolt.", m_name);
			bolt(m_idx, GF_COLD,
			     BO_COLD_DMG(rlev, RANDOMISE));
			update_smart_learn(m_idx, DRS_RES_COLD);
			break;
		}

		case SPELL(1,RSF1_BO_POIS):
		{
			/* XXX XXX XXX */
			break;
		}

		case SPELL(1,RSF1_BO_NETH):
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s casts a nether bolt.", m_name);
			bolt(m_idx, GF_NETHER,
			     BO_NETH_DMG(rlev, RANDOMISE));
			update_smart_learn(m_idx, DRS_RES_NETHR);
			break;
		}

		case SPELL(1,RSF1_BO_WATE):
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s casts a water bolt.", m_name);
			bolt(m_idx, GF_WATER,
			     BO_WATE_DMG(rlev, RANDOMISE));
			break;
		}

		case SPELL(1,RSF1_BO_MANA):
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s casts a mana bolt.", m_name);
			bolt(m_idx, GF_MANA,
			     BO_MANA_DMG(rlev, RANDOMISE));
			break;
		}

		case SPELL(1,RSF1_BO_PLAS):
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s casts a plasma bolt.", m_name);
			bolt(m_idx, GF_PLASMA,
			     BO_PLAS_DMG(rlev, RANDOMISE));
			break;
		}

		case SPELL(1,RSF1_BO_ICEE):
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s casts an ice bolt.", m_name);
			bolt(m_idx, GF_ICE,
			     BO_ICEE_DMG(rlev, RANDOMISE));
			update_smart_learn(m_idx, DRS_RES_COLD);
			break;
		}

		case SPELL(1,RSF1_MISSILE):
		{
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s casts a magic missile.", m_name);
			bolt(m_idx, GF_MISSILE, MISSILE_DMG(rlev, RANDOMISE));
			break;
		}

		case SPELL(1,RSF1_SCARE):
		{
			if (!direct) break;
			disturb(1, 0);
			sound(MSG_CAST_FEAR);
			if (blind) msg_format("%^s mumbles, and you hear scary noises.", m_name);
			else msg_format("%^s casts a fearful illusion.", m_name);
			if (p_ptr->state.resist_fear)
			{
				msg_print("You refuse to be frightened.");
				wieldeds_notice_flag(1, TR1_RES_FEAR);
			}
			else if (randint0(100) < p_ptr->state.skills[SKILL_SAVE])
			{
				msg_print("You refuse to be frightened.");
			}
			else
			{
				(void)inc_timed(TMD_AFRAID, randint0(4) + 4, TRUE);
			}
			update_smart_learn(m_idx, DRS_RES_FEAR);
			break;
		}

		case SPELL(1,RSF1_BLIND):
		{
			if (!direct) break;
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s casts a spell, burning your eyes!", m_name);
			if (p_ptr->state.resist_blind)
			{
				msg_print("You are unaffected!");
				wieldeds_notice_flag(1, TR1_RES_BLIND);
			}
			else if (randint0(100) < p_ptr->state.skills[SKILL_SAVE])
			{
				msg_print("You resist the effects!");
			}
			else
			{
				(void)set_timed(TMD_BLIND, 12 + randint0(4), TRUE);
			}
			update_smart_learn(m_idx, DRS_RES_BLIND);
			break;
		}

		case SPELL(1,RSF1_CONF):
		{
			if (!direct) break;
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles, and you hear puzzling noises.", m_name);
			else msg_format("%^s creates a mesmerising illusion.", m_name);
			if (p_ptr->state.resist_confu)
			{
				msg_print("You disbelieve the feeble spell.");
				wieldeds_notice_flag(1, TR1_RES_CONFU);
			}
			else if (randint0(100) < p_ptr->state.skills[SKILL_SAVE])
			{
				msg_print("You disbelieve the feeble spell.");
			}
			else
			{
				(void)inc_timed(TMD_CONFUSED, randint0(4) + 4, TRUE);
			}
			update_smart_learn(m_idx, DRS_RES_CONFU);
			break;
		}

		case SPELL(1,RSF1_SLOW):
		{
			if (!direct) break;
			disturb(1, 0);
			msg_format("%^s drains power from your muscles!", m_name);
			if (p_ptr->state.free_act)
			{
				msg_print("You are unaffected!");
				wieldeds_notice_flag(2, TR2_FREE_ACT);
			}
			else if (randint0(100) < p_ptr->state.skills[SKILL_SAVE])
			{
				msg_print("You resist the effects!");
			}
			else
			{
				(void)inc_timed(TMD_SLOW, randint0(4) + 4, TRUE);
			}
			update_smart_learn(m_idx, DRS_FREE);
			break;
		}

		case SPELL(1,RSF1_HOLD):
		{
			if (!direct) break;
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s stares deep into your eyes!", m_name);
			if (p_ptr->state.free_act)
			{
				msg_print("You are unaffected!");
				wieldeds_notice_flag(2, TR2_FREE_ACT);
			}
			else if (randint0(100) < p_ptr->state.skills[SKILL_SAVE])
			{
				msg_format("You resist the effects!");
			}
			else
			{
				(void)inc_timed(TMD_PARALYZED, randint0(4) + 4, TRUE);
			}
			update_smart_learn(m_idx, DRS_FREE);
			break;
		}


		case SPELL(2,RSF2_HASTE):
		{
			disturb(1, 0);
			if (blind)
			{
				msg_format("%^s mumbles.", m_name);
			}
			else
			{
				msg_format("%^s concentrates on %s body.", m_name, m_poss);
			}

			/* Allow quick speed increases to base+10 */
			if (m_ptr->mspeed < r_ptr->speed + 10)
			{
				msg_format("%^s starts moving faster.", m_name);
				m_ptr->mspeed += 10;
			}

			/* Allow small speed increases to base+20 */
			else if (m_ptr->mspeed < r_ptr->speed + 20)
			{
				msg_format("%^s starts moving faster.", m_name);
				m_ptr->mspeed += 2;
			}

			break;
		}

		case SPELL(2,RSF2_XXX1):
		{
			break;
		}

		case SPELL(2,RSF2_HEAL):
		{
			disturb(1, 0);

			/* Message */
			if (blind)
			{
				msg_format("%^s mumbles.", m_name);
			}
			else
			{
				msg_format("%^s concentrates on %s wounds.", m_name, m_poss);
			}

			/* Heal some */
			m_ptr->hp += (rlev * 6);

			/* Fully healed */
			if (m_ptr->hp >= m_ptr->maxhp)
			{
				/* Fully healed */
				m_ptr->hp = m_ptr->maxhp;

				/* Message */
				if (seen)
				{
					msg_format("%^s looks REALLY healthy!", m_name);
				}
				else
				{
					msg_format("%^s sounds REALLY healthy!", m_name);
				}
			}

			/* Partially healed */
			else
			{
				/* Message */
				if (seen)
				{
					msg_format("%^s looks healthier.", m_name);
				}
				else
				{
					msg_format("%^s sounds healthier.", m_name);
				}
			}

			/* Redraw (later) if needed */
			if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

			/* Cancel fear */
			if (m_ptr->monfear)
			{
				/* Cancel fear */
				m_ptr->monfear = 0;

				/* Message */
				msg_format("%^s recovers %s courage.", m_name, m_poss);
			}

			break;
		}

		case SPELL(2,RSF2_XXX2):
		{
			break;
		}

		case SPELL(2,RSF2_BLINK):
		{
			disturb(1, 0);
			msg_format("%^s blinks away.", m_name);
			teleport_away(m_idx, 10);
			break;
		}

		case SPELL(2,RSF2_TPORT):
		{
			disturb(1, 0);
			msg_format("%^s teleports away.", m_name);
			teleport_away(m_idx, MAX_SIGHT * 2 + 5);
			break;
		}

		case SPELL(2,RSF2_XXX3):
		{
			break;
		}

		case SPELL(2,RSF2_XXX4):
		{
			break;
		}

		case SPELL(2,RSF2_TELE_TO):
		{
			if (!direct) break;
			disturb(1, 0);
			msg_format("%^s commands you to return.", m_name);
			teleport_player_to(m_ptr->fy, m_ptr->fx);
			break;
		}

		case SPELL(2,RSF2_TELE_AWAY):
		{
			if (!direct) break;
			disturb(1, 0);
			msg_format("%^s teleports you away.", m_name);
			teleport_player(100);
			break;
		}

		case SPELL(2,RSF2_TELE_LEVEL):
		{
			if (!direct) break;
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles strangely.", m_name);
			else msg_format("%^s gestures at your feet.", m_name);
			if (p_ptr->state.resist_nexus)
			{
				msg_print("You are unaffected!");
				wieldeds_notice_flag(1, TR1_RES_NEXUS);
			}
			else if (randint0(100) < p_ptr->state.skills[SKILL_SAVE])
			{
				msg_print("You resist the effects!");
			}
			else
			{
				teleport_player_level();
			}
			update_smart_learn(m_idx, DRS_RES_NEXUS);
			break;
		}

		case SPELL(2,RSF2_XXX5):
		{
			break;
		}

		case SPELL(2,RSF2_DARKNESS):
		{
			if (!direct) break;
			disturb(1, 0);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s gestures in shadow.", m_name);
			(void)unlite_area(0, 3);
			break;
		}

		case SPELL(2,RSF2_TRAPS):
		{
			if (!direct) break;
			disturb(1, 0);
			sound(MSG_CREATE_TRAP);
			if (blind) msg_format("%^s mumbles, and then cackles evilly.", m_name);
			else msg_format("%^s casts a spell and cackles evilly.", m_name);
			(void)trap_creation();
			break;
		}

		case SPELL(2,RSF2_FORGET):
		{
			if (!direct) break;
			disturb(1, 0);
			msg_format("%^s tries to blank your mind.", m_name);

			if (randint0(100) < p_ptr->state.skills[SKILL_SAVE])
				msg_print("You resist the effects!");
			else
				inc_timed(TMD_AMNESIA, 3, TRUE);

			break;
		}

		case SPELL(2,RSF2_XXX6):
		{
			break;
		}

		case SPELL(2,RSF2_S_KIN):
		{
			disturb(1, 0);
			sound(MSG_SUM_MONSTER);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons %s %s.", m_name, m_poss,
			                ((r_ptr->flags[0]) & RF0_UNIQUE ?
			                 "minions" : "kin"));

			/* Hack -- Set the letter of the monsters to summon */
			summon_kin_type = r_ptr->d_char;
			for (k = 0; k < 6; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_KIN, 0);
			}
			if (blind && count)
			{
				msg_print("You hear many things appear nearby.");
			}
			break;
		}

		case SPELL(2,RSF2_S_HI_DEMON):
		{
			disturb(1, 0);
			sound(MSG_SUM_HI_DEMON);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons greater demons!", m_name);
			for (k = 0; k < 8; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_HI_DEMON, 0);
			}
			if (blind && count)
			{
				msg_print("You hear many evil things appear nearby.");
			}
			break;
		}

		case SPELL(2,RSF2_S_MONSTER):
		{
			disturb(1, 0);
			sound(MSG_SUM_MONSTER);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons help!", m_name);
			for (k = 0; k < 1; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_MONSTER, 0);
			}
			if (blind && count)
			{
				msg_print("You hear something appear nearby.");
			}
			break;
		}

		case SPELL(2,RSF2_S_MONSTERS):
		{
			disturb(1, 0);
			sound(MSG_SUM_MONSTER);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons monsters!", m_name);
			for (k = 0; k < 8; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_MONSTERS, 0);
			}
			if (blind && count)
			{
				msg_print("You hear many things appear nearby.");
			}
			break;
		}

		case SPELL(2,RSF2_S_ANIMAL):
		{
			disturb(1, 0);
			sound(MSG_SUM_ANIMAL);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons animals.", m_name);
			for (k = 0; k < 6; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_ANIMAL, 0);
			}
			if (blind && count)
			{
				msg_print("You hear many things appear nearby.");
			}
			break;
		}

		case SPELL(2,RSF2_S_SPIDER):
		{
			disturb(1, 0);
			sound(MSG_SUM_SPIDER);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons spiders.", m_name);
			for (k = 0; k < 6; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_SPIDER, 0);
			}
			if (blind && count)
			{
				msg_print("You hear many things appear nearby.");
			}
			break;
		}

		case SPELL(2,RSF2_S_HOUND):
		{
			disturb(1, 0);
			sound(MSG_SUM_HOUND);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons hounds.", m_name);
			for (k = 0; k < 6; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_HOUND, 0);
			}
			if (blind && count)
			{
				msg_print("You hear many things appear nearby.");
			}
			break;
		}

		case SPELL(2,RSF2_S_HYDRA):
		{
			disturb(1, 0);
			sound(MSG_SUM_HYDRA);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons hydras.", m_name);
			for (k = 0; k < 6; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_HYDRA, 0);
			}
			if (blind && count)
			{
				msg_print("You hear many things appear nearby.");
			}
			break;
		}

		case SPELL(2,RSF2_S_ANGEL):
		{
			disturb(1, 0);
			sound(MSG_SUM_ANGEL);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons an angel!", m_name);
			for (k = 0; k < 1; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_ANGEL, 0);
			}
			if (blind && count)
			{
				msg_print("You hear something appear nearby.");
			}
			break;
		}

		case SPELL(2,RSF2_S_DEMON):
		{
			disturb(1, 0);
			sound(MSG_SUM_DEMON);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons a hellish adversary!", m_name);
			for (k = 0; k < 1; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_DEMON, 0);
			}
			if (blind && count)
			{
				msg_print("You hear something appear nearby.");
			}
			break;
		}

		case SPELL(2,RSF2_S_UNDEAD):
		{
			disturb(1, 0);
			sound(MSG_SUM_UNDEAD);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons an undead adversary!", m_name);
			for (k = 0; k < 1; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_UNDEAD, 0);
			}
			if (blind && count)
			{
				msg_print("You hear something appear nearby.");
			}
			break;
		}

		case SPELL(2,RSF2_S_DRAGON):
		{
			disturb(1, 0);
			sound(MSG_SUM_DRAGON);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons a dragon!", m_name);
			for (k = 0; k < 1; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_DRAGON, 0);
			}
			if (blind && count)
			{
				msg_print("You hear something appear nearby.");
			}
			break;
		}

		case SPELL(2,RSF2_S_HI_UNDEAD):
		{
			disturb(1, 0);
			sound(MSG_SUM_HI_UNDEAD);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons greater undead!", m_name);
			for (k = 0; k < 8; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_HI_UNDEAD, 0);
			}
			if (blind && count)
			{
				msg_print("You hear many creepy things appear nearby.");
			}
			break;
		}

		case SPELL(2,RSF2_S_HI_DRAGON):
		{
			disturb(1, 0);
			sound(MSG_SUM_HI_DRAGON);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons ancient dragons!", m_name);
			for (k = 0; k < 8; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_HI_DRAGON, 0);
			}
			if (blind && count)
			{
				msg_print("You hear many powerful things appear nearby.");
			}
			break;
		}

		case SPELL(2,RSF2_S_WRAITH):
		{
			disturb(1, 0);
			sound(MSG_SUM_WRAITH);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons mighty undead opponents!", m_name);
			for (k = 0; k < 8; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_WRAITH, 0);
			}
			for (k = 0; k < 8; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_HI_UNDEAD, 0);
			}
			if (blind && count)
			{
				msg_print("You hear many creepy things appear nearby.");
			}
			break;
		}

		case SPELL(2,RSF2_S_UNIQUE):
		{
			disturb(1, 0);
			sound(MSG_SUM_UNIQUE);
			if (blind) msg_format("%^s mumbles.", m_name);
			else msg_format("%^s magically summons special opponents!", m_name);
			for (k = 0; k < 8; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_UNIQUE, 0);
			}
			for (k = 0; k < 8; k++)
			{
				count += summon_specific(m_ptr->fy, m_ptr->fx, rlev, SUMMON_HI_UNDEAD, 0);
			}
			if (blind && count)
			{
				msg_print("You hear many powerful things appear nearby.");
			}
			break;
		}
	}


	/* Remember what the monster did to us */
	if (seen)
	{
		SET_FLAG(l_ptr->spell_flags, thrown_spell-SPELL_ORIGIN);

		/* Innate spell */
		if (thrown_spell < SPELL_ORIGIN+32)
		{
			if (l_ptr->cast_innate < MAX_UCHAR) l_ptr->cast_innate++;
		}

		/* Bolt or Ball, or Special spell */
		else if (thrown_spell < SPELL_ORIGIN+32*(RACE_FLAG_SPELL_STRICT_UB))
		{
			if (l_ptr->cast_spell < MAX_UCHAR) l_ptr->cast_spell++;
		}
	}


	/* Always take note of monsters that kill you */
	if (p_ptr->is_dead && (l_ptr->deaths < MAX_SHORT))
	{
		l_ptr->deaths++;
	}


	/* A spell was cast */
	return (TRUE);
}



/*
 * Returns whether a given monster will try to run from the player.
 *
 * Monsters will attempt to avoid very powerful players.  See below.
 *
 * Because this function is called so often, little details are important
 * for efficiency.  Like not using "mod" or "div" when possible.  And
 * attempting to check the conditions in an optimal order.  Note that
 * "(x << 2) == (x * 4)" if "x" has enough bits to hold the result.
 *
 * Note that this function is responsible for about one to five percent
 * of the processor use in normal conditions...
 */
static int mon_will_run(int m_idx)
{
	monster_type *m_ptr = &mon_list[m_idx];

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	u16b p_lev, m_lev;
	u16b p_chp, p_mhp;
	u16b m_chp, m_mhp;
	u32b p_val, m_val;

	/* Keep monsters from running too far away */
	if (m_ptr->cdis > MAX_SIGHT + 5) return (FALSE);

	/* All "afraid" monsters will run away */
	if (m_ptr->monfear) return (TRUE);

	/* Nearby monsters will not become terrified */
	if (m_ptr->cdis <= 5) return (FALSE);

	/* Examine player power (level) */
	p_lev = p_ptr->lev;

	/* Examine monster power (level plus morale) */
	m_lev = r_ptr->level + (m_idx & 0x08) + 25;

	/* Optimize extreme cases below */
	if (m_lev > p_lev + 4) return (FALSE);
	if (m_lev + 4 <= p_lev) return (TRUE);

	/* Examine player health */
	p_chp = p_ptr->chp;
	p_mhp = p_ptr->mhp;

	/* Examine monster health */
	m_chp = m_ptr->hp;
	m_mhp = m_ptr->maxhp;

	/* Prepare to optimize the calculation */
	p_val = (p_lev * p_mhp) + (p_chp << 2);	/* div p_mhp */
	m_val = (m_lev * m_mhp) + (m_chp << 2);	/* div m_mhp */

	/* Strong players scare strong monsters */
	if (p_val * m_mhp > m_val * p_mhp) return (TRUE);

	/* Assume no terror */
	return (FALSE);
}




/*
 * Choose the "best" direction for "flowing"
 *
 * Note that ghosts and rock-eaters are never allowed to "flow",
 * since they should move directly towards the player.
 *
 * Prefer "non-diagonal" directions, but twiddle them a little
 * to angle slightly towards the player's actual location.
 *
 * Allow very perceptive monsters to track old "spoor" left by
 * previous locations occupied by the player.  This will tend
 * to have monsters end up either near the player or on a grid
 * recently occupied by the player (and left via "teleport").
 *
 * Note that if "smell" is turned on, all monsters get vicious.
 *
 * Also note that teleporting away from a location will cause
 * the monsters who were chasing you to converge on that location
 * as long as you are still near enough to "annoy" them without
 * being close enough to chase directly.  I have no idea what will
 * happen if you combine "smell" with low "aaf" values.
 */
static bool get_moves_aux(int m_idx, int *yp, int *xp)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int i, y, x, y1, x1;

	int when = 0;
	int cost = 999;

	monster_type *m_ptr = &mon_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	/* Monster flowing disabled */
	if (!OPT(adult_ai_sound)) return (FALSE);

	/* Monster can go through rocks */
	if (r_ptr->flags[1] & (RF1_PASS_WALL | RF1_KILL_WALL)) return (FALSE);

	/* Monster location */
	y1 = m_ptr->fy;
	x1 = m_ptr->fx;

	/* The player is not currently near the monster grid */
	if (cave_when[y1][x1] < cave_when[py][px])
	{
		/* The player has never been near the monster grid */
		if (cave_when[y1][x1] == 0) return (FALSE);

		/* The monster is not allowed to track the player */
		if (!OPT(adult_ai_smell)) return (FALSE);
	}

	/* Monster is too far away to notice the player */
	if (cave_cost[y1][x1] > MONSTER_FLOW_DEPTH) return (FALSE);
	if (cave_cost[y1][x1] > r_ptr->aaf) return (FALSE);

	/* Hack -- Player can see us, run towards him */
	if (player_has_los_bold(y1, x1)) return (FALSE);

	/* Check nearby grids, diagonals first */
	for (i = 7; i >= 0; i--)
	{
		/* Get the location */
		y = y1 + ddy_ddd[i];
		x = x1 + ddx_ddd[i];

		/* Ignore illegal locations */
		if (cave_when[y][x] == 0) continue;

		/* Ignore ancient locations */
		if (cave_when[y][x] < when) continue;

		/* Ignore distant locations */
		if (cave_cost[y][x] > cost) continue;

		/* Save the cost and time */
		when = cave_when[y][x];
		cost = cave_cost[y][x];

		/* Hack -- Save the "twiddled" location */
		(*yp) = py + 16 * ddy_ddd[i];
		(*xp) = px + 16 * ddx_ddd[i];
	}

	/* No legal move (?) */
	if (!when) return (FALSE);

	/* Success */
	return (TRUE);
}

/*
 * Provide a location to flee to, but give the player a wide berth.
 *
 * A monster may wish to flee to a location that is behind the player,
 * but instead of heading directly for it, the monster should "swerve"
 * around the player so that he has a smaller chance of getting hit.
 */
static bool get_fear_moves_aux(int m_idx, int *yp, int *xp)
{
	int y, x, y1, x1, fy, fx, py, px, gy = 0, gx = 0;
	int when = 0, score = -1;
	int i;

	monster_type *m_ptr = &mon_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	/* Monster flowing disabled */
	if (!OPT(adult_ai_sound)) return (FALSE);

	/* Player location */
	py = p_ptr->py;
	px = p_ptr->px;

	/* Monster location */
	fy = m_ptr->fy;
	fx = m_ptr->fx;

	/* Desired destination */
	y1 = fy - (*yp);
	x1 = fx - (*xp);

	/* The player is not currently near the monster grid */
	if (cave_when[fy][fx] < cave_when[py][px])
	{
		/* No reason to attempt flowing */
		return (FALSE);
	}

	/* Monster is too far away to use flow information */
	if (cave_cost[fy][fx] > MONSTER_FLOW_DEPTH) return (FALSE);
	if (cave_cost[fy][fx] > r_ptr->aaf) return (FALSE);

	/* Check nearby grids, diagonals first */
	for (i = 7; i >= 0; i--)
	{
		int dis, s;

		/* Get the location */
		y = fy + ddy_ddd[i];
		x = fx + ddx_ddd[i];

		/* Ignore illegal locations */
		if (cave_when[y][x] == 0) continue;

		/* Ignore ancient locations */
		if (cave_when[y][x] < when) continue;

		/* Calculate distance of this grid from our destination */
		dis = distance(y, x, y1, x1);

		/* Score this grid */
		s = 5000 / (dis + 3) - 500 / (cave_cost[y][x] + 1);

		/* No negative scores */
		if (s < 0) s = 0;

		/* Ignore lower scores */
		if (s < score) continue;

		/* Save the score and time */
		when = cave_when[y][x];
		score = s;

		/* Save the location */
		gy = y;
		gx = x;
	}

	/* No legal move (?) */
	if (!when) return (FALSE);

	/* Find deltas */
	(*yp) = fy - gy;
	(*xp) = fx - gx;

	/* Success */
	return (TRUE);
}



/*
 * Hack -- Precompute a bunch of calls to distance() in find_safety() and
 * find_hiding().
 *
 * The pair of arrays dist_offsets_y[n] and dist_offsets_x[n] contain the
 * offsets of all the locations with a distance of n from a central point,
 * with an offset of (0,0) indicating no more offsets at this distance.
 *
 * This is, of course, fairly unreadable, but it eliminates multiple loops
 * from the previous version.
 *
 * It is probably better to replace these arrays with code to compute
 * the relevant arrays, even if the storage is pre-allocated in hard
 * coded sizes.  At the very least, code should be included which is
 * able to generate and dump these arrays (ala "los()").  XXX XXX XXX
 *
 * Also, the storage needs could be reduced by using char.  XXX XXX XXX
 *
 * These arrays could be combined into two big arrays, using sub-arrays
 * to hold the offsets and lengths of each portion of the sub-arrays, and
 * this could perhaps also be used somehow in the "look" code.  XXX XXX XXX
 */


static const int d_off_y_0[] =
{ 0 };

static const int d_off_x_0[] =
{ 0 };


static const int d_off_y_1[] =
{ -1, -1, -1, 0, 0, 1, 1, 1, 0 };

static const int d_off_x_1[] =
{ -1, 0, 1, -1, 1, -1, 0, 1, 0 };


static const int d_off_y_2[] =
{ -1, -1, -2, -2, -2, 0, 0, 1, 1, 2, 2, 2, 0 };

static const int d_off_x_2[] =
{ -2, 2, -1, 0, 1, -2, 2, -2, 2, -1, 0, 1, 0 };


static const int d_off_y_3[] =
{ -1, -1, -2, -2, -3, -3, -3, 0, 0, 1, 1, 2, 2,
  3, 3, 3, 0 };

static const int d_off_x_3[] =
{ -3, 3, -2, 2, -1, 0, 1, -3, 3, -3, 3, -2, 2,
  -1, 0, 1, 0 };


static const int d_off_y_4[] =
{ -1, -1, -2, -2, -3, -3, -3, -3, -4, -4, -4, 0,
  0, 1, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 0 };

static const int d_off_x_4[] =
{ -4, 4, -3, 3, -2, -3, 2, 3, -1, 0, 1, -4, 4,
  -4, 4, -3, 3, -2, -3, 2, 3, -1, 0, 1, 0 };


static const int d_off_y_5[] =
{ -1, -1, -2, -2, -3, -3, -4, -4, -4, -4, -5, -5,
  -5, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 4, 5, 5,
  5, 0 };

static const int d_off_x_5[] =
{ -5, 5, -4, 4, -4, 4, -2, -3, 2, 3, -1, 0, 1,
  -5, 5, -5, 5, -4, 4, -4, 4, -2, -3, 2, 3, -1,
  0, 1, 0 };


static const int d_off_y_6[] =
{ -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -5, -5,
  -6, -6, -6, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5,
  5, 5, 6, 6, 6, 0 };

static const int d_off_x_6[] =
{ -6, 6, -5, 5, -5, 5, -4, 4, -2, -3, 2, 3, -1,
  0, 1, -6, 6, -6, 6, -5, 5, -5, 5, -4, 4, -2,
  -3, 2, 3, -1, 0, 1, 0 };


static const int d_off_y_7[] =
{ -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -5, -5,
  -6, -6, -6, -6, -7, -7, -7, 0, 0, 1, 1, 2, 2, 3,
  3, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 0 };

static const int d_off_x_7[] =
{ -7, 7, -6, 6, -6, 6, -5, 5, -4, -5, 4, 5, -2,
  -3, 2, 3, -1, 0, 1, -7, 7, -7, 7, -6, 6, -6,
  6, -5, 5, -4, -5, 4, 5, -2, -3, 2, 3, -1, 0,
  1, 0 };


static const int d_off_y_8[] =
{ -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -6, -6,
  -6, -6, -7, -7, -7, -7, -8, -8, -8, 0, 0, 1, 1,
  2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7,
  8, 8, 8, 0 };

static const int d_off_x_8[] =
{ -8, 8, -7, 7, -7, 7, -6, 6, -6, 6, -4, -5, 4,
  5, -2, -3, 2, 3, -1, 0, 1, -8, 8, -8, 8, -7,
  7, -7, 7, -6, 6, -6, 6, -4, -5, 4, 5, -2, -3,
  2, 3, -1, 0, 1, 0 };


static const int d_off_y_9[] =
{ -1, -1, -2, -2, -3, -3, -4, -4, -5, -5, -6, -6,
  -7, -7, -7, -7, -8, -8, -8, -8, -9, -9, -9, 0,
  0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 7,
  7, 8, 8, 8, 8, 9, 9, 9, 0 };

static const int d_off_x_9[] =
{ -9, 9, -8, 8, -8, 8, -7, 7, -7, 7, -6, 6, -4,
  -5, 4, 5, -2, -3, 2, 3, -1, 0, 1, -9, 9, -9,
  9, -8, 8, -8, 8, -7, 7, -7, 7, -6, 6, -4, -5,
  4, 5, -2, -3, 2, 3, -1, 0, 1, 0 };


static const int *dist_offsets_y[10] =
{
	d_off_y_0, d_off_y_1, d_off_y_2, d_off_y_3, d_off_y_4,
	d_off_y_5, d_off_y_6, d_off_y_7, d_off_y_8, d_off_y_9
};

static const int *dist_offsets_x[10] =
{
	d_off_x_0, d_off_x_1, d_off_x_2, d_off_x_3, d_off_x_4,
	d_off_x_5, d_off_x_6, d_off_x_7, d_off_x_8, d_off_x_9
};


/*
 * Choose a "safe" location near a monster for it to run toward.
 *
 * A location is "safe" if it can be reached quickly and the player
 * is not able to fire into it (it isn't a "clean shot").  So, this will
 * cause monsters to "duck" behind walls.  Hopefully, monsters will also
 * try to run towards corridor openings if they are in a room.
 *
 * This function may take lots of CPU time if lots of monsters are fleeing.
 *
 * Return TRUE if a safe location is available.
 */
static bool find_safety(int m_idx, int *yp, int *xp)
{
	monster_type *m_ptr = &mon_list[m_idx];

	int fy = m_ptr->fy;
	int fx = m_ptr->fx;

	int py = p_ptr->py;
	int px = p_ptr->px;

	int i, y, x, dy, dx, d, dis;
	int gy = 0, gx = 0, gdis = 0;

	const int *y_offsets;
	const int *x_offsets;

	/* Start with adjacent locations, spread further */
	for (d = 1; d < 10; d++)
	{
		/* Get the lists of points with a distance d from (fx, fy) */
		y_offsets = dist_offsets_y[d];
		x_offsets = dist_offsets_x[d];

		/* Check the locations */
		for (i = 0, dx = x_offsets[0], dy = y_offsets[0];
		     dx != 0 || dy != 0;
		     i++, dx = x_offsets[i], dy = y_offsets[i])
		{
			y = fy + dy;
			x = fx + dx;

			/* Skip illegal locations */
			if (!in_bounds_fully(y, x)) continue;

			/* Skip locations in a wall */
			if (!cave_floor_bold(y, x)) continue;

			/* Check for "availability" (if monsters can flow) */
			if (OPT(adult_ai_sound))
			{
				/* Ignore grids very far from the player */
				if (cave_when[y][x] < cave_when[py][px]) continue;

				/* Ignore too-distant grids */
				if (cave_cost[y][x] > cave_cost[fy][fx] + 2 * d) continue;
			}

			/* Check for absence of shot (more or less) */
			if (!player_has_los_bold(y,x))
			{
				/* Calculate distance from player */
				dis = distance(y, x, py, px);

				/* Remember if further than previous */
				if (dis > gdis)
				{
					gy = y;
					gx = x;
					gdis = dis;
				}
			}
		}

		/* Check for success */
		if (gdis > 0)
		{
			/* Good location */
			(*yp) = fy - gy;
			(*xp) = fx - gx;

			/* Found safe place */
			return (TRUE);
		}
	}

	/* No safe place */
	return (FALSE);
}




/*
 * Choose a good hiding place near a monster for it to run toward.
 *
 * Pack monsters will use this to "ambush" the player and lure him out
 * of corridors into open space so they can swarm him.
 *
 * Return TRUE if a good location is available.
 */
static bool find_hiding(int m_idx, int *yp, int *xp)
{
	monster_type *m_ptr = &mon_list[m_idx];

	int fy = m_ptr->fy;
	int fx = m_ptr->fx;

	int py = p_ptr->py;
	int px = p_ptr->px;

	int i, y, x, dy, dx, d, dis;
	int gy = 0, gx = 0, gdis = 999, min;

	const int *y_offsets, *x_offsets;

	/* Closest distance to get */
	min = distance(py, px, fy, fx) * 3 / 4 + 2;

	/* Start with adjacent locations, spread further */
	for (d = 1; d < 10; d++)
	{
		/* Get the lists of points with a distance d from (fx, fy) */
		y_offsets = dist_offsets_y[d];
		x_offsets = dist_offsets_x[d];

		/* Check the locations */
		for (i = 0, dx = x_offsets[0], dy = y_offsets[0];
		     dx != 0 || dy != 0;
		     i++, dx = x_offsets[i], dy = y_offsets[i])
		{
			y = fy + dy;
			x = fx + dx;

			/* Skip illegal locations */
			if (!in_bounds_fully(y, x)) continue;

			/* Skip occupied locations */
			if (!cave_empty_bold(y, x)) continue;

			/* Check for hidden, available grid */
			if (!player_has_los_bold(y, x) && (clean_shot(fy, fx, y, x)))
			{
				/* Calculate distance from player */
				dis = distance(y, x, py, px);

				/* Remember if closer than previous */
				if (dis < gdis && dis >= min)
				{
					gy = y;
					gx = x;
					gdis = dis;
				}
			}
		}

		/* Check for success */
		if (gdis < 999)
		{
			/* Good location */
			(*yp) = fy - gy;
			(*xp) = fx - gx;

			/* Found good place */
			return (TRUE);
		}
	}

	/* No good place */
	return (FALSE);
}


/*
 * Choose "logical" directions for monster movement
 *
 * We store the directions in a special "mm" array
 */
static bool get_moves(int m_idx, int mm[5])
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	monster_type *m_ptr = &mon_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	int y, ay, x, ax;

	int move_val = 0;

	int y2 = py;
	int x2 = px;

	bool done = FALSE;

	/* Flow towards the player */
	if (OPT(adult_ai_sound))
	{
		/* Flow towards the player */
		(void)get_moves_aux(m_idx, &y2, &x2);
	}

	/* Extract the "pseudo-direction" */
	y = m_ptr->fy - y2;
	x = m_ptr->fx - x2;



	/* Normal animal packs try to get the player out of corridors. */
	if (OPT(adult_ai_packs) &&
	    (r_ptr->flags[0] & RF0_FRIENDS) && (r_ptr->flags[2] & RF2_ANIMAL) &&
	    !((r_ptr->flags[1] & (RF1_PASS_WALL | RF1_KILL_WALL))))
	{
		int i, open = 0;

		/* Count empty grids next to player */
		for (i = 0; i < 8; i++)
		{
			/* Check grid around the player for room interior (room walls count)
			   or other empty space */
			if ((cave_feat[py + ddy_ddd[i]][px + ddx_ddd[i]] <= FEAT_MORE) ||
				(cave_info[py + ddy_ddd[i]][px + ddx_ddd[i]] & (CAVE_ROOM)))
			{
				/* One more open grid */
				open++;
			}
		}

		/* Not in an empty space and strong player */
		if ((open < 7) && (p_ptr->chp > p_ptr->mhp / 2))
		{
			/* Find hiding place */
			if (find_hiding(m_idx, &y, &x)) done = TRUE;
		}
	}


	/* Apply fear */
	if (!done && mon_will_run(m_idx))
	{
		/* Try to find safe place */
		if (!(OPT(adult_ai_smart) && find_safety(m_idx, &y, &x)))
		{
			/* This is not a very "smart" method XXX XXX */
			y = (-y);
			x = (-x);
		}

		else
		{
			/* Attempt to avoid the player */
			if (OPT(adult_ai_sound))
			{
				/* Adjust movement */
				get_fear_moves_aux(m_idx, &y, &x);
			}
		}

		done = TRUE;
	}


	/* Monster groups try to surround the player */
	if (!done && OPT(adult_ai_packs) && (r_ptr->flags[0] & RF0_FRIENDS))
	{
		int i;

		/* If we are not already adjacent */
		if (m_ptr->cdis > 1)
		{
			/* Find an empty square near the player to fill */
			int tmp = randint0(8);
			for (i = 0; i < 8; i++)
			{
				/* Pick squares near player (pseudo-randomly) */
				y2 = py + ddy_ddd[(tmp + i) & 7];
				x2 = px + ddx_ddd[(tmp + i) & 7];
				
				/* Ignore filled grids */
				if (!cave_empty_bold(y2, x2)) continue;
				
				/* Try to fill this hole */
				break;
			}
		}
		/* Extract the new "pseudo-direction" */
		y = m_ptr->fy - y2;
		x = m_ptr->fx - x2;
	}


	/* Check for no move */
	if (!x && !y) return (FALSE);

	/* Extract the "absolute distances" */
	ax = ABS(x);
	ay = ABS(y);

	/* Do something weird */
	if (y < 0) move_val += 8;
	if (x > 0) move_val += 4;

	/* Prevent the diamond maneuvre */
	if (ay > (ax << 1))
	{
		move_val++;
		move_val++;
	}
	else if (ax > (ay << 1))
	{
		move_val++;
	}

	/* Analyze */
	switch (move_val)
	{
		case 0:
		{
			mm[0] = 9;
			if (ay > ax)
			{
				mm[1] = 8;
				mm[2] = 6;
				mm[3] = 7;
				mm[4] = 3;
			}
			else
			{
				mm[1] = 6;
				mm[2] = 8;
				mm[3] = 3;
				mm[4] = 7;
			}
			break;
		}

		case 1:
		case 9:
		{
			mm[0] = 6;
			if (y < 0)
			{
				mm[1] = 3;
				mm[2] = 9;
				mm[3] = 2;
				mm[4] = 8;
			}
			else
			{
				mm[1] = 9;
				mm[2] = 3;
				mm[3] = 8;
				mm[4] = 2;
			}
			break;
		}

		case 2:
		case 6:
		{
			mm[0] = 8;
			if (x < 0)
			{
				mm[1] = 9;
				mm[2] = 7;
				mm[3] = 6;
				mm[4] = 4;
			}
			else
			{
				mm[1] = 7;
				mm[2] = 9;
				mm[3] = 4;
				mm[4] = 6;
			}
			break;
		}

		case 4:
		{
			mm[0] = 7;
			if (ay > ax)
			{
				mm[1] = 8;
				mm[2] = 4;
				mm[3] = 9;
				mm[4] = 1;
			}
			else
			{
				mm[1] = 4;
				mm[2] = 8;
				mm[3] = 1;
				mm[4] = 9;
			}
			break;
		}

		case 5:
		case 13:
		{
			mm[0] = 4;
			if (y < 0)
			{
				mm[1] = 1;
				mm[2] = 7;
				mm[3] = 2;
				mm[4] = 8;
			}
			else
			{
				mm[1] = 7;
				mm[2] = 1;
				mm[3] = 8;
				mm[4] = 2;
			}
			break;
		}

		case 8:
		{
			mm[0] = 3;
			if (ay > ax)
			{
				mm[1] = 2;
				mm[2] = 6;
				mm[3] = 1;
				mm[4] = 9;
			}
			else
			{
				mm[1] = 6;
				mm[2] = 2;
				mm[3] = 9;
				mm[4] = 1;
			}
			break;
		}

		case 10:
		case 14:
		{
			mm[0] = 2;
			if (x < 0)
			{
				mm[1] = 3;
				mm[2] = 1;
				mm[3] = 6;
				mm[4] = 4;
			}
			else
			{
				mm[1] = 1;
				mm[2] = 3;
				mm[3] = 4;
				mm[4] = 6;
			}
			break;
		}

		default: /* case 12: */
		{
			mm[0] = 1;
			if (ay > ax)
			{
				mm[1] = 2;
				mm[2] = 4;
				mm[3] = 3;
				mm[4] = 7;
			}
			else
			{
				mm[1] = 4;
				mm[2] = 2;
				mm[3] = 7;
				mm[4] = 3;
			}
			break;
		}
	}

	/* Want to move */
	return (TRUE);
}



/*
 * Hack -- compare the "strength" of two monsters XXX XXX XXX
 */
static int compare_monsters(const monster_type *m_ptr, const monster_type *n_ptr)
{
	monster_race *r_ptr;

	u32b mexp1, mexp2;

	/* Race 1 */
	r_ptr = &r_info[m_ptr->r_idx];

	/* Extract mexp */
	mexp1 = r_ptr->mexp;

	/* Race 2 */
	r_ptr = &r_info[n_ptr->r_idx];

	/* Extract mexp */
	mexp2 = r_ptr->mexp;

	/* Compare */
	if (mexp1 < mexp2) return (-1);
	if (mexp1 > mexp2) return (1);

	/* Assume equal */
	return (0);
}


/*
 * Process a monster
 *
 * In several cases, we directly update the monster lore
 *
 * Note that a monster is only allowed to "reproduce" if there
 * are a limited number of "reproducing" monsters on the current
 * level.  This should prevent the level from being "swamped" by
 * reproducing monsters.  It also allows a large mass of mice to
 * prevent a louse from multiplying, but this is a small price to
 * pay for a simple multiplication method.
 *
 * XXX Monster fear is slightly odd, in particular, monsters will
 * fixate on opening a door even if they cannot open it.  Actually,
 * the same thing happens to normal monsters when they hit a door
 *
 * In addition, monsters which *cannot* open or bash down a door
 * will still stand there trying to open it...  XXX XXX XXX
 *
 * Technically, need to check for monster in the way combined
 * with that monster being in a wall (or door?) XXX
 */
static void process_monster(int m_idx)
{
	monster_type *m_ptr = &mon_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	monster_lore *l_ptr = &l_list[m_ptr->r_idx];

	int i, d, oy, ox, ny, nx;

	int mm[5];

	bool woke_up = FALSE;
	bool stagger;

	bool do_turn;
	bool do_move;
	bool do_view;

	bool did_open_door;
	bool did_bash_door;


	/* Handle "sleep" */
	if (m_ptr->csleep)
	{
		u32b notice;

		/* Aggravation */
		if (p_ptr->state.aggravate)
		{
			/* Reset sleep counter */
			woke_up = wake_monster(m_ptr);

			/* Notice the "waking up" */
			if (m_ptr->ml)
			{
				char m_name[80];

				/* Get the monster name */
				monster_desc(m_name, sizeof(m_name), m_ptr, 0);

				/* Dump a message */
				msg_format("%^s wakes up.", m_name);

				/* Hack -- Update the health bar */
				if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);
			}

			/* Efficiency XXX XXX */
			return;
		}

		/* Anti-stealth */
		notice = randint0(1024);

		/* Hack -- See if monster "notices" player */
		if ((notice * notice * notice) <= p_ptr->state.noise)
		{
			int d = 1;

			/* Wake up faster near the player */
			if (m_ptr->cdis < 50) d = (100 / m_ptr->cdis);

			/* Still asleep */
			if (m_ptr->csleep > d)
			{
				/* Monster wakes up "a little bit" */
				m_ptr->csleep -= d;

				/* Notice the "not waking up" */
				if (m_ptr->ml)
				{
					/* Hack -- Count the ignores */
					if (l_ptr->ignore < MAX_UCHAR)
					{
						l_ptr->ignore++;
					}
				}
			}

			/* Just woke up */
			else
			{
				/* Reset sleep counter */
				woke_up = wake_monster(m_ptr);

				/* Notice the "waking up" */
				if (m_ptr->ml)
				{
					char m_name[80];

					/* Get the monster name */
					monster_desc(m_name, sizeof(m_name), m_ptr, 0);

					/* Dump a message */
					msg_format("%^s wakes up.", m_name);

					/* Hack -- Update the health bar */
					if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

					/* Hack -- Count the wakings */
					if (l_ptr->wake < MAX_UCHAR)
					{
						l_ptr->wake++;
					}
				}
			}
		}

		/* Still sleeping */
		if (m_ptr->csleep) return;
	}

	/* If the monster just woke up, then it doesn't act */
	if (woke_up) return;

	/* Handle "stun" */
	if (m_ptr->stunned)
	{
		int d = 1;

		/* Make a "saving throw" against stun */
		if (randint0(5000) <= r_ptr->level * r_ptr->level)
		{
			/* Recover fully */
			d = m_ptr->stunned;
		}

		/* Hack -- Recover from stun */
		if (m_ptr->stunned > d)
		{
			/* Recover somewhat */
			m_ptr->stunned -= d;
		}

		/* Fully recover */
		else
		{
			/* Recover fully */
			m_ptr->stunned = 0;

			/* Message if visible */
			if (m_ptr->ml)
			{
				char m_name[80];

				/* Get the monster name */
				monster_desc(m_name, sizeof(m_name), m_ptr, 0);

				/* Dump a message */
				msg_format("%^s is no longer stunned.", m_name);

				/* Hack -- Update the health bar */
				if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);
			}
		}

		/* Still stunned */
		if (m_ptr->stunned) return;
	}


	/* Handle confusion */
	if (m_ptr->confused)
	{
		int d = randint1(r_ptr->level / 10 + 1);

		/* Still confused */
		if (m_ptr->confused > d)
		{
			/* Reduce the confusion */
			m_ptr->confused -= d;
		}

		/* Recovered */
		else
		{
			/* No longer confused */
			m_ptr->confused = 0;

			/* Message if visible */
			if (m_ptr->ml)
			{
				char m_name[80];

				/* Get the monster name */
				monster_desc(m_name, sizeof(m_name), m_ptr, 0);

				/* Dump a message */
				msg_format("%^s is no longer confused.", m_name);

				/* Hack -- Update the health bar */
				if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);
			}
		}
	}


	/* Handle "fear" */
	if (m_ptr->monfear)
	{
		/* Amount of "boldness" */
		int d = randint1(r_ptr->level / 10 + 1);

		/* Still afraid */
		if (m_ptr->monfear > d)
		{
			/* Reduce the fear */
			m_ptr->monfear -= d;
		}

		/* Recover from fear, take note if seen */
		else
		{
			/* No longer afraid */
			m_ptr->monfear = 0;

			/* Visual note */
			if (m_ptr->ml)
			{
				char m_name[80];
				char m_poss[80];

				/* Get the monster name/poss */
				monster_desc(m_name, sizeof(m_name), m_ptr, 0);
				monster_desc(m_poss, sizeof(m_poss), m_ptr, MDESC_PRO2 | MDESC_POSS);

				/* Dump a message */
				msg_format("%^s recovers %s courage.", m_name, m_poss);

				/* Hack -- Update the health bar */
				if (p_ptr->health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);
			}
		}
	}


	/* Get the origin */
	oy = m_ptr->fy;
	ox = m_ptr->fx;


	/* Attempt to "mutiply" (all monsters are allowed an attempt for lore
	 * purposes, even non-breeders)
	 */
	if (num_repro < MAX_REPRO)
	{
		int k, y, x;

		/* Count the adjacent monsters */
		for (k = 0, y = oy - 1; y <= oy + 1; y++)
		{
			for (x = ox - 1; x <= ox + 1; x++)
			{
				/* Count monsters */
				if (cave_m_idx[y][x] > 0) k++;
			}
		}

		/* Multiply slower in crowded areas */
		if ((k < 4) && (k == 0 || one_in_(k * MON_MULT_ADJ)))
		{
			/* Successful breeding attempt, learn about that now */
			if (m_ptr->ml) l_ptr->flags[1] |= (RF1_MULTIPLY);
			
			/* Try to multiply (only breeders allowed) */
			if ((r_ptr->flags[1] & (RF1_MULTIPLY)) && multiply_monster(m_idx))
			{
				/* Make a sound */
				if (m_ptr->ml)
				{
					sound(MSG_MULTIPLY);
				}

				/* Multiplying takes energy */
				return;
			}
		}
	}


	/* Attempt to cast a spell */
	if (make_attack_spell(m_idx)) return;


	/* Reset */
	stagger = FALSE;

	/* Confused */
	if (m_ptr->confused)
	{
		/* Stagger */
		stagger = TRUE;
	}

	/* Random movement - always attempt for lore purposes */
	else
	{
		int roll = randint0(100);
		
		/* Random movement (25%) */
		if (roll < 25)
		{
			/* Learn about small random movement */
			if (m_ptr->ml) l_ptr->flags[0] |= (RF0_RAND_25);

			/* Stagger */			
			if (r_ptr->flags[0] & (RF0_RAND_25 | RF0_RAND_50)) stagger = TRUE;
		}
		
		/* Random movement (50%) */
		else if (roll < 50)
		{
			/* Learn about medium random movement */
			if (m_ptr->ml) l_ptr->flags[0] |= (RF0_RAND_50);

			/* Stagger */			
			if (r_ptr->flags[0] & (RF0_RAND_50)) stagger = TRUE;
		}
		
		/* Random movement (75%) */
		else if (roll < 75)
		{
			/* Stagger */			
			if (r_ptr->flags[0] & (RF0_RAND_25) &&
			    r_ptr->flags[0] & (RF0_RAND_50))
			{
				stagger = TRUE;
			}
		}
	}

	/* Normal movement */
	if (!stagger)
	{
		/* Logical moves, may do nothing */
		if (!get_moves(m_idx, mm)) return;
	}


	/* Assume nothing */
	do_turn = FALSE;
	do_move = FALSE;
	do_view = FALSE;

	/* Assume nothing */
	did_open_door = FALSE;
	did_bash_door = FALSE;


	/* Process moves */
	for (i = 0; i < 5; i++)
	{
		/* Get the direction (or stagger) */
		d = (stagger ? ddd[randint0(8)] : mm[i]);

		/* Get the destination */
		ny = oy + ddy[d];
		nx = ox + ddx[d];


		/* Floor is open? */
		if (cave_floor_bold(ny, nx))
		{
			/* Go ahead and move */
			do_move = TRUE;
		}

		/* Permanent wall in the way */
		else if (cave_feat[ny][nx] >= FEAT_PERM_EXTRA)
		{
			/* Nothing */
		}

		/* Normal wall, door, or secret door in the way */
		else
		{
			/* There's some kind of feature in the way, so learn about
			 * kill-wall and pass-wall now
			 */
			if (m_ptr->ml) l_ptr->flags[1] |= (RF1_PASS_WALL | RF1_KILL_WALL);

			/* Monster moves through walls (and doors) */
			if (r_ptr->flags[1] & (RF1_PASS_WALL))
			{
				/* Pass through walls/doors/rubble */
				do_move = TRUE;
			}

			/* Monster destroys walls (and doors) */
			else if (r_ptr->flags[1] & (RF1_KILL_WALL))
			{
				/* Eat through walls/doors/rubble */
				do_move = TRUE;

				/* Forget the wall */
				cave_info[ny][nx] &= ~(CAVE_MARK);

				/* Notice */
				cave_set_feat(ny, nx, FEAT_FLOOR);

				/* Note changes to viewable region */
				if (player_has_los_bold(ny, nx)) do_view = TRUE;
			}

			/* Handle doors and secret doors */
			else if (((cave_feat[ny][nx] >= FEAT_DOOR_HEAD) &&
						 (cave_feat[ny][nx] <= FEAT_DOOR_TAIL)) ||
						(cave_feat[ny][nx] == FEAT_SECRET))
			{
				bool may_bash = TRUE;

				/* Take a turn */
				do_turn = TRUE;
				
				/* Learn about door abilities */
				if (m_ptr->ml) l_ptr->flags[1] |= (RF1_OPEN_DOOR | RF1_BASH_DOOR);

				/* Creature can open doors. */
				if (r_ptr->flags[1] & (RF1_OPEN_DOOR))
				{
					/* Closed doors and secret doors */
					if ((cave_feat[ny][nx] == FEAT_DOOR_HEAD) ||
						 (cave_feat[ny][nx] == FEAT_SECRET))
					{
						/* The door is open */
						did_open_door = TRUE;

						/* Do not bash the door */
						may_bash = FALSE;
					}

					/* Locked doors (not jammed) */
					else if (cave_feat[ny][nx] < FEAT_DOOR_HEAD + 0x08)
					{
						int k;

						/* Door power */
						k = ((cave_feat[ny][nx] - FEAT_DOOR_HEAD) & 0x07);

						/* Try to unlock it XXX XXX XXX */
						if (randint0(m_ptr->hp / 10) > k)
						{
							/* Unlock the door */
							cave_set_feat(ny, nx, FEAT_DOOR_HEAD + 0x00);

							/* Do not bash the door */
							may_bash = FALSE;
						}
					}
				}

				/* Stuck doors -- attempt to bash them down if allowed */
				if (may_bash && (r_ptr->flags[1] & (RF1_BASH_DOOR)))
				{
					int k;

					/* Door power */
					k = ((cave_feat[ny][nx] - FEAT_DOOR_HEAD) & 0x07);

					/* Attempt to Bash XXX XXX XXX */
					if (randint0(m_ptr->hp / 10) > k)
					{
						/* Message */
						msg_print("You hear a door burst open!");

						/* Disturb (sometimes) */
						disturb(0, 0);

						/* The door was bashed open */
						did_bash_door = TRUE;

						/* Hack -- fall into doorway */
						do_move = TRUE;
					}
				}
			}

			/* Deal with doors in the way */
			if (did_open_door || did_bash_door)
			{
				/* Break down the door */
				if (did_bash_door && (randint0(100) < 50))
				{
					cave_set_feat(ny, nx, FEAT_BROKEN);
				}

				/* Open the door */
				else
				{
					cave_set_feat(ny, nx, FEAT_OPEN);
				}

				/* Handle viewable doors */
				if (player_has_los_bold(ny, nx)) do_view = TRUE;
			}
		}


		/* Hack -- check for Glyph of Warding */
		if (do_move && (cave_feat[ny][nx] == FEAT_GLYPH))
		{
			/* Assume no move allowed */
			do_move = FALSE;

			/* Break the ward */
			if (randint1(BREAK_GLYPH) < r_ptr->level)
			{
				/* Describe observable breakage */
				if (cave_info[ny][nx] & (CAVE_MARK))
				{
					msg_print("The rune of protection is broken!");
				}

				/* Forget the rune */
				cave_info[ny][nx] &= ~(CAVE_MARK);

				/* Break the rune */
				cave_set_feat(ny, nx, FEAT_FLOOR);

				/* Allow movement */
				do_move = TRUE;
			}
		}


		/* The player is in the way. */
		if (do_move && (cave_m_idx[ny][nx] < 0))
		{
			/* Learn about if the monster attacks */
			if (m_ptr->ml) l_ptr->flags[0] |= (RF0_NEVER_BLOW);

			/* Some monsters never attack */
			if (r_ptr->flags[0] & (RF0_NEVER_BLOW))
			{
				/* Do not move */
				do_move = FALSE;
			}
			
			/* Otherwise, attack the player */
			else
			{
				/* Do the attack */
				(void)make_attack_normal(m_idx);

				/* Do not move */
				do_move = FALSE;

				/* Took a turn */
				do_turn = TRUE;
			}
		}


		/* Some monsters never move */
		if (do_move && (r_ptr->flags[0] & (RF0_NEVER_MOVE)))
		{
			/* Learn about lack of movement */
			if (m_ptr->ml) l_ptr->flags[0] |= (RF0_NEVER_MOVE);

			/* Do not move */
			do_move = FALSE;
		}


		/* A monster is in the way */
		if (do_move && (cave_m_idx[ny][nx] > 0))
		{
			monster_type *n_ptr = &mon_list[cave_m_idx[ny][nx]];

			/* Kill weaker monsters */
			int kill_ok = (r_ptr->flags[1] & RF1_KILL_BODY);

			/* Move weaker monsters if they can swap places */
			/* (not in a wall) */
			int move_ok = (r_ptr->flags[1] & RF1_MOVE_BODY &&
						   cave_floor_bold(m_ptr->fy, m_ptr->fx));

			/* Assume no movement */
			do_move = FALSE;

			if (compare_monsters(m_ptr, n_ptr) > 0)
			{
				/* Learn about pushing and shoving */
				if (m_ptr->ml) l_ptr->flags[1] |= (RF1_KILL_BODY | RF1_MOVE_BODY);

				if (kill_ok || move_ok)
				{
					/* Get the names of the monsters involved */
					char m_name[80];
					char n_name[80];
					monster_desc(m_name, sizeof(m_name), m_ptr, MDESC_IND1);
					monster_desc(n_name, sizeof(n_name), n_ptr, MDESC_IND1);

					/* Allow movement */
					do_move = TRUE;

					/* Monster ate another monster */
					if (kill_ok)
					{
						/* Note if visible */
						if (m_ptr->ml && (m_ptr->mflag & (MFLAG_VIEW)))
						{
							msg_format("%^s tramples over %s.", m_name, n_name);
						}

						delete_monster(ny, nx);
					}
					else
					{
						/* Note if visible */
						if (m_ptr->ml && (m_ptr->mflag & (MFLAG_VIEW)))
						{
							msg_format("%^s pushes past %s.", m_name, n_name);
						}
					}
				}
			}
		}

		/* Creature has been allowed move */
		if (do_move)
		{
			s16b this_o_idx, next_o_idx = 0;
			
			/* Learn about no lack of movement */
			if (m_ptr->ml) l_ptr->flags[0] |= (RF0_NEVER_MOVE);

			/* Take a turn */
			do_turn = TRUE;

			/* Move the monster */
			monster_swap(oy, ox, ny, nx);

			/* Possible disturb */
			if (m_ptr->ml &&
			    (OPT(disturb_move) ||
			     ((m_ptr->mflag & (MFLAG_VIEW)) &&
			      OPT(disturb_near))))
			{
				/* Disturb */
				disturb(0, 0);
			}


			/* Scan all objects in the grid */
			for (this_o_idx = cave_o_idx[ny][nx]; this_o_idx; this_o_idx = next_o_idx)
			{
				object_type *o_ptr;

				/* Get the object */
				o_ptr = &o_list[this_o_idx];

				/* Get the next object */
				next_o_idx = o_ptr->next_o_idx;

				/* Skip gold */
				if (o_ptr->tval == TV_GOLD) continue;
				
				/* Learn about item pickup behavior */
				if (m_ptr->ml) l_ptr->flags[1] |= (RF1_TAKE_ITEM | RF1_KILL_ITEM);

				/* Take or Kill objects on the floor */
				if ((r_ptr->flags[1] & (RF1_TAKE_ITEM)) ||
				    (r_ptr->flags[1] & (RF1_KILL_ITEM)))
				{
					u32b f[OBJ_FLAG_N];

					u32b flg2 = 0L;

					char m_name[80];
					char o_name[80];

					/* Extract some flags */
					object_flags(o_ptr, f);

					/* Get the object name */
					object_desc(o_name, sizeof(o_name), o_ptr,
								ODESC_PREFIX | ODESC_FULL);

					/* Get the monster name */
					monster_desc(m_name, sizeof(m_name), m_ptr, MDESC_IND1);

					/* React to objects that hurt the monster */
					if (f[0] & (TR0_KILL_DRAGON)) flg2 |= (RF2_DRAGON);
					if (f[0] & (TR0_KILL_DEMON)) flg2 |= (RF2_DEMON);
					if (f[0] & (TR0_KILL_UNDEAD)) flg2 |= (RF2_UNDEAD);
					if (f[0] & (TR0_SLAY_DRAGON)) flg2 |= (RF2_DRAGON);
					if (f[0] & (TR0_SLAY_TROLL)) flg2 |= (RF2_TROLL);
					if (f[0] & (TR0_SLAY_GIANT)) flg2 |= (RF2_GIANT);
					if (f[0] & (TR0_SLAY_ORC)) flg2 |= (RF2_ORC);
					if (f[0] & (TR0_SLAY_DEMON)) flg2 |= (RF2_DEMON);
					if (f[0] & (TR0_SLAY_UNDEAD)) flg2 |= (RF2_UNDEAD);
					if (f[0] & (TR0_SLAY_ANIMAL)) flg2 |= (RF2_ANIMAL);
					if (f[0] & (TR0_SLAY_EVIL)) flg2 |= (RF2_EVIL);

					/* The object cannot be picked up by the monster */
					if (artifact_p(o_ptr) || (r_ptr->flags[2] & flg2))
					{
						/* Only give a message for "take_item" */
						if (r_ptr->flags[1] & (RF1_TAKE_ITEM))
						{
							/* Describe observable situations */
							if (m_ptr->ml && player_has_los_bold(ny, nx) && !squelch_hide_item(o_ptr))
							{
								/* Dump a message */
								msg_format("%^s tries to pick up %s, but fails.",
								           m_name, o_name);
							}
						}
					}

					/* Pick up the item */
					else if (r_ptr->flags[1] & (RF1_TAKE_ITEM))
					{
						object_type *i_ptr;
						object_type object_type_body;

						/* Describe observable situations */
						if (player_has_los_bold(ny, nx) && !squelch_hide_item(o_ptr))
						{
							/* Dump a message */
							msg_format("%^s picks up %s.", m_name, o_name);
						}

						/* Get local object */
						i_ptr = &object_type_body;

						/* Obtain local object */
						object_copy(i_ptr, o_ptr);

						/* Delete the object */
						delete_object_idx(this_o_idx);

						/* Carry the object */
						(void)monster_carry(m_idx, i_ptr);
					}

					/* Destroy the item */
					else
					{
						/* Describe observable situations */
						if (player_has_los_bold(ny, nx) && !squelch_hide_item(o_ptr))
						{
							/* Dump a message */
							message_format(MSG_DESTROY, 0, "%^s crushes %s.", m_name, o_name);
						}

						/* Delete the object */
						delete_object_idx(this_o_idx);
					}
				}
			}
		}

		/* Stop when done */
		if (do_turn) break;
	}


	/* If we haven't done anything, try casting a spell again */
	if (OPT(adult_ai_smart) && !do_turn && !do_move)
	{
		/* Cast spell */
		if (make_attack_spell(m_idx)) return;
	}

	/* Notice changes in view */
	if (do_view)
	{
		/* Update the visuals */
		p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

		/* Fully update the flow XXX XXX XXX */
		p_ptr->update |= (PU_FORGET_FLOW | PU_UPDATE_FLOW);
	}


	/* Hack -- get "bold" if out of options */
	if (!do_turn && !do_move && m_ptr->monfear)
	{
		/* No longer afraid */
		m_ptr->monfear = 0;

		/* Message if seen */
		if (m_ptr->ml)
		{
			char m_name[80];

			/* Get the monster name */
			monster_desc(m_name, sizeof(m_name), m_ptr, 0);

			/* Dump a message */
			msg_format("%^s turns to fight!", m_name);
		}

		/* XXX XXX XXX Actually do something now (?) */
	}
}


static bool monster_can_flow(int m_idx)
{
	/* Hack -- Monsters can "smell" the player from far away */
	if (OPT(adult_ai_sound))
	{
		monster_type *m_ptr = &mon_list[m_idx];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		/* Monster location */
		int fy = m_ptr->fy;
		int fx = m_ptr->fx;

		/* Check the flow (normal aaf is about 20) */
		if ((cave_when[fy][fx] == cave_when[p_ptr->py][p_ptr->px]) &&
		    (cave_cost[fy][fx] < MONSTER_FLOW_DEPTH) &&
		    (cave_cost[fy][fx] < r_ptr->aaf))
		{
			return TRUE;
		}
	}

	return FALSE;
}




/*
 * Process all the "live" monsters, once per game turn.
 *
 * During each game turn, we scan through the list of all the "live" monsters,
 * (backwards, so we can excise any "freshly dead" monsters), energizing each
 * monster, and allowing fully energized monsters to move, attack, pass, etc.
 *
 * Note that monsters can never move in the monster array (except when the
 * "compact_monsters()" function is called by "dungeon()" or "save_player()").
 *
 * This function is responsible for at least half of the processor time
 * on a normal system with a "normal" amount of monsters and a player doing
 * normal things.
 *
 * When the player is resting, virtually 90% of the processor time is spent
 * in this function, and its children, "process_monster()" and "make_move()".
 *
 * Most of the rest of the time is spent in "update_view()" and "lite_spot()",
 * especially when the player is running.
 *
 * Note the special "MFLAG_NICE" flag, which prevents "nasty" monsters from
 * using any of their spell attacks until the player gets a turn.  This flag
 * is optimized via the "repair_mflag_nice" flag.
 */
void process_monsters(byte minimum_energy)
{
	int i;

	monster_type *m_ptr;
	monster_race *r_ptr;

	/* Process the monsters (backwards) */
	for (i = mon_max - 1; i >= 1; i--)
	{
		/* Handle "leaving" */
		if (p_ptr->leaving) break;


		/* Get the monster */
		m_ptr = &mon_list[i];


		/* Ignore "dead" monsters */
		if (!m_ptr->r_idx) continue;


		/* Not enough energy to move */
		if (m_ptr->energy < minimum_energy) continue;

		/* Use up "some" energy */
		m_ptr->energy -= 100;


		/* Heal monster? XXX XXX XXX */


		/* Get the race */
		r_ptr = &r_info[m_ptr->r_idx];

		/*
		 * Process the monster if the monster either:
		 * - can "sense" the player
		 * - is hurt
		 * - can "see" the player (checked backwards)
		 * - can "smell" the player from far away (flow)
		 */
		if ((m_ptr->cdis <= r_ptr->aaf) ||
		    (m_ptr->hp < m_ptr->maxhp) ||
		    player_has_los_bold(m_ptr->fy, m_ptr->fx) ||
		    monster_can_flow(i))
		{
			/* Process the monster */
			process_monster(i);
		}
	}
}
