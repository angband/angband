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
#include "attack.h"
#include "cave.h"
#include "monster/mon-spell.h"
#include "object/slays.h"
#include "object/tvalsval.h"
#include "spells.h"
#include "squelch.h"

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
 * to remove attacks or spells before using them. 
 *
 * This has the added advantage that attacks and spells are related.
 * The "smart_learn" option means that the monster "learns" the flags
 * that should be set, and "smart_cheat" means that he "knows" them.
 * So "smart_cheat" means that the "smart" field is always up to date,
 * while "smart_learn" means that the "smart" field is slowly learned.
 * Both of them have the same effect on the "choose spell" routine.
 */

/*
 * Remove the "bad" spells from a spell list
 */
static void remove_bad_spells(int m_idx, bitflag f[RSF_SIZE])
{
	monster_type *m_ptr = cave_monster(cave, m_idx);
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	bitflag f2[RSF_SIZE], ai_flags[OF_SIZE];

	size_t i;	
	u32b smart = 0L;

	/* Stupid monsters act randomly */
	if (rf_has(r_ptr->flags, RF_STUPID)) return;

	/* Take working copy of spell flags */
	rsf_copy(f2, f);

	/* Don't heal if full */
	if (m_ptr->hp >= m_ptr->maxhp) rsf_off(f2, RSF_HEAL);
	
	/* Don't haste if hasted with time remaining */
	if (m_ptr->m_timed[MON_TMD_FAST] > 10) rsf_off(f2, RSF_HASTE);

	/* Don't teleport to if the player is already next to us */
	if (m_ptr->cdis == 1) rsf_off(f2, RSF_TELE_TO);

	/* Update acquired knowledge */
	of_wipe(ai_flags);
	if (OPT(birth_ai_learn))
	{
		/* Occasionally forget player status */
		if (one_in_(100))
			of_wipe(m_ptr->known_pflags);

		/* Use the memorized flags */
		smart = m_ptr->smart;
		of_copy(ai_flags, m_ptr->known_pflags);
	}

	/* Cheat if requested */
	if (OPT(birth_ai_cheat)) {
		for (i = 0; i < OF_MAX; i++)
			if (check_state(p_ptr, i, p_ptr->state.flags))
				of_on(ai_flags, i);
		if (!p_ptr->msp) smart |= SM_IMM_MANA;
	}

	/* Cancel out certain flags based on knowledge */
	if (!of_is_empty(ai_flags))
		unset_spells(f2, ai_flags, r_ptr);

	if (smart & SM_IMM_MANA && randint0(100) <
			50 * (rf_has(r_ptr->flags, RF_SMART) ? 2 : 1))
		rsf_off(f2, RSF_DRAIN_MANA);

	/* use working copy of spell flags */
	rsf_copy(f, f2);
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
			if (cave->feat[y][x] == FEAT_GLYPH) continue;

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
static int choose_attack_spell(int m_idx, bitflag f[RSF_SIZE])
{
	monster_type *m_ptr = cave_monster(cave, m_idx);
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	int num = 0;
	byte spells[RSF_MAX];

	int i, py = p_ptr->py, px = p_ptr->px;

	bool has_escape, has_attack, has_summon, has_tactic;
	bool has_annoy, has_haste, has_heal;


	/* Smart monsters restrict their spell choices. */
	if (OPT(birth_ai_smart) && !rf_has(r_ptr->flags, RF_STUPID))
	{
		/* What have we got? */
		has_escape = test_spells(f, RST_ESCAPE);
		has_attack = test_spells(f, RST_ATTACK | RST_BOLT | RST_BALL | RST_BREATH);
		has_summon = test_spells(f, RST_SUMMON);
		has_tactic = test_spells(f, RST_TACTIC);
		has_annoy = test_spells(f, RST_ANNOY);
		has_haste = test_spells(f, RST_HASTE);
		has_heal = test_spells(f, RST_HEAL);

		/*** Try to pick an appropriate spell type ***/

		/* Hurt badly or afraid, attempt to flee */
		if (has_escape && ((m_ptr->hp < m_ptr->maxhp / 4) || m_ptr->m_timed[MON_TMD_FEAR]))
		{
			/* Choose escape spell */
			set_spells(f, RST_ESCAPE);
		}

		/* Still hurt badly, couldn't flee, attempt to heal */
		else if (has_heal && m_ptr->hp < m_ptr->maxhp / 4)
		{
			/* Choose heal spell */
			set_spells(f, RST_HEAL);
		}

		/* Player is close and we have attack spells, blink away */
		else if (has_tactic && (distance(py, px, m_ptr->fy, m_ptr->fx) < 4) &&
		         has_attack && (randint0(100) < 75))
		{
			/* Choose tactical spell */
			set_spells(f, RST_TACTIC);
		}

		/* We're hurt (not badly), try to heal */
		else if (has_heal && (m_ptr->hp < m_ptr->maxhp * 3 / 4) &&
		         (randint0(100) < 60))
		{
			/* Choose heal spell */
			set_spells(f, RST_HEAL);
		}

		/* Summon if possible (sometimes) */
		else if (has_summon && (randint0(100) < 50))
		{
			/* Choose summon spell */
			set_spells(f, RST_SUMMON);
		}

		/* Attack spell (most of the time) */
		else if (has_attack && (randint0(100) < 85))
		{
			/* Choose attack spell */
			set_spells(f, RST_ATTACK | RST_BOLT | RST_BALL | RST_BREATH);
		}

		/* Try another tactical spell (sometimes) */
		else if (has_tactic && (randint0(100) < 50))
		{
			/* Choose tactic spell */
			set_spells(f, RST_TACTIC);
		}

		/* Haste self if we aren't already somewhat hasted (rarely) */
		else if (has_haste && (randint0(100) < (20 + r_ptr->speed - m_ptr->mspeed)))
		{
			/* Choose haste spell */
			set_spells(f, RST_HASTE);
		}

		/* Annoy player (most of the time) */
		else if (has_annoy && (randint0(100) < 85))
		{
			/* Choose annoyance spell */
			set_spells(f, RST_ANNOY);
		}

		/* Else choose no spell */
		else
		{
			rsf_wipe(f);
		}

		/* Anything left? */
		if (rsf_is_empty(f)) return (FLAG_END);
	}

	/* Extract all spells: "innate", "normal", "bizarre" */
	for (i = FLAG_START, num = 0; i < RSF_MAX; i++)
	{
		if (rsf_has(f, i)) spells[num++] = i;
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
	int chance, thrown_spell, rlev, failrate;

	bitflag f[RSF_SIZE];

	monster_type *m_ptr = cave_monster(cave, m_idx);
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	monster_lore *l_ptr = &l_list[m_ptr->r_idx];

	char m_name[80], m_poss[80], ddesc[80];

	/* Player position */
	int px = p_ptr->px;
	int py = p_ptr->py;

	/* Extract the blind-ness */
	bool blind = (p_ptr->timed[TMD_BLIND] ? TRUE : FALSE);

	/* Extract the "see-able-ness" */
	bool seen = (!blind && m_ptr->ml);

	/* Assume "normal" target */
	bool normal = TRUE;

	/* Handle "leaving" */
	if (p_ptr->leaving) return FALSE;

	/* Cannot cast spells when confused */
	if (m_ptr->m_timed[MON_TMD_CONF]) return (FALSE);

	/* Cannot cast spells when nice */
	if (m_ptr->mflag & MFLAG_NICE) return FALSE;

	/* Hack -- Extract the spell probability */
	chance = (r_ptr->freq_innate + r_ptr->freq_spell) / 2;

	/* Not allowed to cast spells */
	if (!chance) return FALSE;

	/* Only do spells occasionally */
	if (randint0(100) >= chance) return FALSE;

	/* Hack -- require projectable player */
	if (normal)
	{
		/* Check range */
		if (m_ptr->cdis > MAX_RANGE) return FALSE;

		/* Check path */
		if (!projectable(m_ptr->fy, m_ptr->fx, py, px, PROJECT_NONE))
			return FALSE;
	}

	/* Extract the monster level */
	rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);

	/* Extract the racial spell flags */
	rsf_copy(f, r_ptr->spell_flags);

	/* Allow "desperate" spells */
	if (rf_has(r_ptr->flags, RF_SMART) &&
	    m_ptr->hp < m_ptr->maxhp / 10 &&
	    randint0(100) < 50)

		/* Require intelligent spells */
		set_spells(f, RST_HASTE | RST_ANNOY | RST_ESCAPE | RST_HEAL | RST_TACTIC | RST_SUMMON);

	/* Remove the "ineffective" spells */
	remove_bad_spells(m_idx, f);

	/* Check whether summons and bolts are worth it. */
	if (!rf_has(r_ptr->flags, RF_STUPID))
	{
		/* Check for a clean bolt shot */
		if (test_spells(f, RST_BOLT) &&
			!clean_shot(m_ptr->fy, m_ptr->fx, py, px))

			/* Remove spells that will only hurt friends */
			set_spells(f, ~RST_BOLT);

		/* Check for a possible summon */
		if (!(summon_possible(m_ptr->fy, m_ptr->fx)))

			/* Remove summoning spells */
			set_spells(f, ~RST_SUMMON);
	}

	/* No spells left */
	if (rsf_is_empty(f)) return FALSE;

	/* Get the monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), m_ptr, 0x00);

	/* Get the monster possessive ("his"/"her"/"its") */
	monster_desc(m_poss, sizeof(m_poss), m_ptr, MDESC_PRO2 | MDESC_POSS);

	/* Get the "died from" name */
	monster_desc(ddesc, sizeof(ddesc), m_ptr, MDESC_SHOW | MDESC_IND2);

	/* Choose a spell to cast */
	thrown_spell = choose_attack_spell(m_idx, f);

	/* Abort if no spell was chosen */
	if (!thrown_spell) return FALSE;

	/* If we see an unaware monster try to cast a spell, become aware of it */
	if (m_ptr->unaware)
	{
		m_ptr->unaware = FALSE;

		/* Learn about mimicry */
		if (rf_has(r_ptr->flags, RF_UNAWARE))
			rf_on(l_ptr->flags, RF_UNAWARE);
	}

	/* Calculate spell failure rate */
	failrate = 25 - (rlev + 3) / 4;
	if (m_ptr->m_timed[MON_TMD_FEAR])
		failrate += 20;

	/* Stupid monsters will never fail (for jellies and such) */
	if (OPT(birth_ai_smart) || rf_has(r_ptr->flags, RF_STUPID))
		failrate = 0;

	/* Check for spell failure (innate attacks never fail) */
	if ((thrown_spell >= MIN_NONINNATE_SPELL) && (randint0(100) < failrate))
	{
		/* Message */
		msg("%^s tries to cast a spell, but fails.", m_name);

		return TRUE;
	}

	/* Cast the spell. */
	disturb(p_ptr, 1, 0);

	/* Special case RSF_HASTE until TMD_* and MON_TMD_* are rationalised */
	if (thrown_spell == RSF_HASTE) {
		if (blind)
			msg("%^s mumbles.", m_name);
		else
			msg("%^s concentrates on %s body.", m_name, m_poss);

		/* XXX Allow slow speed increases past +10 */
		if (m_ptr->m_timed[MON_TMD_FAST] &&
				m_ptr->mspeed > r_ptr->speed + 10 &&
				m_ptr->mspeed < r_ptr->speed + 20) {
			msg("%^s starts moving faster.", m_name);
			m_ptr->mspeed += 2;
		}
		(void)mon_inc_timed(m_idx, MON_TMD_FAST, 50, 0);
	} else 
		do_mon_spell(thrown_spell, m_idx, seen);

	/* Remember what the monster did to us */
	if (seen) {
		rsf_on(l_ptr->spell_flags, thrown_spell);

		/* Innate spell */
		if (thrown_spell < MIN_NONINNATE_SPELL) {
			if (l_ptr->cast_innate < MAX_UCHAR)
				l_ptr->cast_innate++;
		} else {
		/* Bolt or Ball, or Special spell */
			if (l_ptr->cast_spell < MAX_UCHAR)
				l_ptr->cast_spell++;
		}
	}
	/* Always take note of monsters that kill you */
	if (p_ptr->is_dead && (l_ptr->deaths < MAX_SHORT)) {
		l_ptr->deaths++;
	}

	/* A spell was cast */
	return TRUE;
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
	monster_type *m_ptr = cave_monster(cave, m_idx);

	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	u16b p_lev, m_lev;
	u16b p_chp, p_mhp;
	u16b m_chp, m_mhp;
	u32b p_val, m_val;

	/* Keep monsters from running too far away */
	if (m_ptr->cdis > MAX_SIGHT + 5) return (FALSE);

	/* All "afraid" monsters will run away */
	if (m_ptr->m_timed[MON_TMD_FEAR]) return (TRUE);

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
static bool get_moves_aux(struct cave *c, int m_idx, int *yp, int *xp)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int i, y, x, y1, x1;

	int when = 0;
	int cost = 999;

	monster_type *m_ptr = cave_monster(cave, m_idx);
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	/* Monster can go through rocks */
	if (flags_test(r_ptr->flags, RF_SIZE, RF_PASS_WALL, RF_KILL_WALL, FLAG_END)) return (FALSE);

	/* Monster location */
	y1 = m_ptr->fy;
	x1 = m_ptr->fx;

	/* The player is not currently near the monster grid */
	if (c->when[y1][x1] < c->when[py][px])
	{
		/* The player has never been near the monster grid */
		if (c->when[y1][x1] == 0) return (FALSE);

		/* The monster is not allowed to track the player */
		if (!OPT(birth_ai_smell)) return (FALSE);
	}

	/* Monster is too far away to notice the player */
	if (c->cost[y1][x1] > MONSTER_FLOW_DEPTH) return (FALSE);
	if (c->cost[y1][x1] > r_ptr->aaf) return (FALSE);

	/* Hack -- Player can see us, run towards him */
	if (player_has_los_bold(y1, x1)) return (FALSE);

	/* Check nearby grids, diagonals first */
	for (i = 7; i >= 0; i--)
	{
		/* Get the location */
		y = y1 + ddy_ddd[i];
		x = x1 + ddx_ddd[i];

		/* Ignore illegal locations */
		if (c->when[y][x] == 0) continue;

		/* Ignore ancient locations */
		if (c->when[y][x] < when) continue;

		/* Ignore distant locations */
		if (c->cost[y][x] > cost) continue;

		/* Save the cost and time */
		when = c->when[y][x];
		cost = c->cost[y][x];

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
static bool get_fear_moves_aux(struct cave *c, int m_idx, int *yp, int *xp)
{
	int y, x, y1, x1, fy, fx, py, px, gy = 0, gx = 0;
	int when = 0, score = -1;
	int i;

	monster_type *m_ptr = cave_monster(cave, m_idx);
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

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
	if (c->when[fy][fx] < c->when[py][px])
	{
		/* No reason to attempt flowing */
		return (FALSE);
	}

	/* Monster is too far away to use flow information */
	if (c->cost[fy][fx] > MONSTER_FLOW_DEPTH) return (FALSE);
	if (c->cost[fy][fx] > r_ptr->aaf) return (FALSE);

	/* Check nearby grids, diagonals first */
	for (i = 7; i >= 0; i--)
	{
		int dis, s;

		/* Get the location */
		y = fy + ddy_ddd[i];
		x = fx + ddx_ddd[i];

		/* Ignore illegal locations */
		if (c->when[y][x] == 0) continue;

		/* Ignore ancient locations */
		if (c->when[y][x] < when) continue;

		/* Calculate distance of this grid from our destination */
		dis = distance(y, x, y1, x1);

		/* Score this grid */
		s = 5000 / (dis + 3) - 500 / (c->cost[y][x] + 1);

		/* No negative scores */
		if (s < 0) s = 0;

		/* Ignore lower scores */
		if (s < score) continue;

		/* Save the score and time */
		when = c->when[y][x];
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
static bool find_safety(struct cave *c, int m_idx, int *yp, int *xp)
{
	monster_type *m_ptr = cave_monster(cave, m_idx);

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

			/* Ignore grids very far from the player */
			if (c->when[y][x] < c->when[py][px]) continue;

			/* Ignore too-distant grids */
			if (c->cost[y][x] > c->cost[fy][fx] + 2 * d) continue;

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
	monster_type *m_ptr = cave_monster(cave, m_idx);

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
static bool get_moves(struct cave *c, int m_idx, int mm[5])
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	monster_type *m_ptr = cave_monster(cave, m_idx);
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	int y, ay, x, ax;

	int move_val = 0;

	int y2 = py;
	int x2 = px;

	bool done = FALSE;

	/* Flow towards the player */
	get_moves_aux(c, m_idx, &y2, &x2);

	/* Extract the "pseudo-direction" */
	y = m_ptr->fy - y2;
	x = m_ptr->fx - x2;



	/* Normal animal packs try to get the player out of corridors. */
	if (OPT(birth_ai_packs) &&
	    rf_has(r_ptr->flags, RF_FRIENDS) && rf_has(r_ptr->flags, RF_ANIMAL) &&
	    !flags_test(r_ptr->flags, RF_SIZE, RF_PASS_WALL, RF_KILL_WALL, FLAG_END))
	{
		int i, open = 0;

		/* Count empty grids next to player */
		for (i = 0; i < 8; i++)
		{
			/* Check grid around the player for room interior (room walls count)
			   or other empty space */
			if ((cave->feat[py + ddy_ddd[i]][px + ddx_ddd[i]] <= FEAT_MORE) ||
				(cave->info[py + ddy_ddd[i]][px + ddx_ddd[i]] & (CAVE_ROOM)))
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
		if (!(OPT(birth_ai_smart) && find_safety(c, m_idx, &y, &x)))
		{
			/* This is not a very "smart" method XXX XXX */
			y = (-y);
			x = (-x);
		}

		else
		{
			/* Adjust movement */
			get_fear_moves_aux(c, m_idx, &y, &x);
		}

		done = TRUE;
	}


	/* Monster groups try to surround the player */
	if (!done && OPT(birth_ai_packs) && rf_has(r_ptr->flags, RF_FRIENDS))
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
 * Critical blow.  All hits that do 95% of total possible damage,
 * and which also do at least 20 damage, or, sometimes, N damage.
 * This is used only to determine "cuts" and "stuns".
 */
static int monster_critical(int dice, int sides, int dam)
{
	int max = 0;
	int total = dice * sides;

	/* Must do at least 95% of perfect */
	if (dam < total * 19 / 20) return (0);

	/* Weak blows rarely work */
	if ((dam < 20) && (randint0(100) >= dam)) return (0);

	/* Perfect damage */
	if (dam == total) max++;

	/* Super-charge */
	if (dam >= 20)
	{
		while (randint0(100) < 2) max++;
	}

	/* Critical damage */
	if (dam > 45) return (6 + max);
	if (dam > 33) return (5 + max);
	if (dam > 25) return (4 + max);
	if (dam > 18) return (3 + max);
	if (dam > 11) return (2 + max);
	return (1 + max);
}

/*
 * Determine if a monster attack against the player succeeds.
 */
bool check_hit(struct player *p, int power, int level)
{
	int chance, ac;

	/* Calculate the "attack quality" */
	chance = (power + (level * 3));

	/* Total armor */
	ac = p->state.ac + p->state.to_a;

	/* if the monster checks vs ac, the player learns ac bonuses */
	/* XXX Eddie should you only learn +ac on miss, -ac on hit?  who knows */
	object_notice_on_defend(p);

	/* Check if the player was hit */
	return test_hit(chance, ac, TRUE);
}


#define MAX_DESC_INSULT 8


/*
 * Hack -- possible "insult" messages
 */
static const char *desc_insult[MAX_DESC_INSULT] =
{
	"insults you!",
	"insults your mother!",
	"gives you the finger!",
	"humiliates you!",
	"defiles you!",
	"dances around you!",
	"makes obscene gestures!",
	"moons you!!!"
};


#define MAX_DESC_MOAN 8


/*
 * Hack -- possible "insult" messages
 */
static const char *desc_moan[MAX_DESC_MOAN] =
{
	"wants his mushrooms back.",
	"tells you to get off his land.",
	"looks for his dogs. ",
	"says 'Did you kill my Fang?' ",
	"asks 'Do you want to buy any mushrooms?' ",
	"seems sad about something.",
	"asks if you have seen his dogs.",
	"mumbles something about mushrooms."
};

/*
 * Attack the player via physical attacks.
 */
static bool make_attack_normal(struct monster *m_ptr, struct player *p)
{
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	monster_lore *l_ptr = &l_list[m_ptr->r_idx];

	int ap_cnt;

	int i, k, tmp, ac, rlev;
	int do_cut, do_stun;

	s32b gold;

	object_type *o_ptr;

	char o_name[80];

	char m_name[80];

	char ddesc[80];

	bool blinked;

	int sound_msg;


	/* Not allowed to attack */
	if (rf_has(r_ptr->flags, RF_NEVER_BLOW)) return (FALSE);

	/* Become aware of monster
	if (m_ptr->unaware)
	{
		m_ptr->unaware = FALSE;
		update_mon(m_idx, FALSE);
	} */

	/* Total armor */
	ac = p->state.ac + p->state.to_a;

	/* Extract the effective monster level */
	rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);


	/* Get the monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), m_ptr, 0);

	/* Get the "died from" information (i.e. "a kobold") */
	monster_desc(ddesc, sizeof(ddesc), m_ptr, MDESC_SHOW | MDESC_IND2);

	/* Assume no blink */
	blinked = FALSE;

	/* Scan through all blows */
	for (ap_cnt = 0; ap_cnt < MONSTER_BLOW_MAX; ap_cnt++)
	{
		bool visible = FALSE;
		bool obvious = FALSE;
		bool do_break = FALSE;

		int power = 0;
		int damage = 0;

		const char *act = NULL;

		/* Extract the attack infomation */
		int effect = r_ptr->blow[ap_cnt].effect;
		int method = r_ptr->blow[ap_cnt].method;
		int d_dice = r_ptr->blow[ap_cnt].d_dice;
		int d_side = r_ptr->blow[ap_cnt].d_side;


		/* Hack -- no more attacks */
		if (!method) break;

		/* Handle "leaving" */
		if (p->leaving) break;

		/* Extract visibility (before blink) */
		if (m_ptr->ml) visible = TRUE;

		/* Extract visibility from carrying light */
		if (rf_has(r_ptr->flags, RF_HAS_LIGHT)) visible = TRUE;

		/* Extract the attack "power" */
		switch (effect)
		{
			case RBE_HURT:      power = 60; break;
			case RBE_POISON:    power =  5; break;
			case RBE_UN_BONUS:  power = 20; break;
			case RBE_UN_POWER:  power = 15; break;
			case RBE_EAT_GOLD:  power =  5; break;
			case RBE_EAT_ITEM:  power =  5; break;
			case RBE_EAT_FOOD:  power =  5; break;
			case RBE_EAT_LIGHT: power =  5; break;
			case RBE_ACID:      power =  0; break;
			case RBE_ELEC:      power = 10; break;
			case RBE_FIRE:      power = 10; break;
			case RBE_COLD:      power = 10; break;
			case RBE_BLIND:     power =  2; break;
			case RBE_CONFUSE:   power = 10; break;
			case RBE_TERRIFY:   power = 10; break;
			case RBE_PARALYZE:  power =  2; break;
			case RBE_LOSE_STR:  power =  0; break;
			case RBE_LOSE_DEX:  power =  0; break;
			case RBE_LOSE_CON:  power =  0; break;
			case RBE_LOSE_INT:  power =  0; break;
			case RBE_LOSE_WIS:  power =  0; break;
			case RBE_LOSE_CHR:  power =  0; break;
			case RBE_LOSE_ALL:  power =  2; break;
			case RBE_SHATTER:   power = 60; break;
			case RBE_EXP_10:    power =  5; break;
			case RBE_EXP_20:    power =  5; break;
			case RBE_EXP_40:    power =  5; break;
			case RBE_EXP_80:    power =  5; break;
			case RBE_HALLU:     power = 10; break;
		}


		/* Monster hits player */
		if (!effect || check_hit(p, power, rlev))
		{
			/* Always disturbing */
			disturb(p, 1, 0);

			/* Hack -- Apply "protection from evil" */
			if (p->timed[TMD_PROTEVIL] > 0)
			{
				/* Learn about the evil flag */
				if (m_ptr->ml)
				{
					rf_on(l_ptr->flags, RF_EVIL);
				}

				if (rf_has(r_ptr->flags, RF_EVIL) &&
				    p->lev >= rlev &&
				    randint0(100) + p->lev > 50)
				{
					/* Message */
					msg("%^s is repelled.", m_name);

					/* Hack -- Next attack */
					continue;
				}
			}


			/* Assume no cut or stun */
			do_cut = do_stun = 0;

			/* Assume no sound */
			sound_msg = MSG_GENERIC;

			/* Describe the attack method */
			switch (method)
			{
				case RBM_HIT:
				{
					act = "hits you.";
					do_cut = do_stun = 1;
					sound_msg = MSG_MON_HIT;
					break;
				}

				case RBM_TOUCH:
				{
					act = "touches you.";
					sound_msg = MSG_MON_TOUCH;
					break;
				}

				case RBM_PUNCH:
				{
					act = "punches you.";
					do_stun = 1;
					sound_msg = MSG_MON_PUNCH;
					break;
				}

				case RBM_KICK:
				{
					act = "kicks you.";
					do_stun = 1;
					sound_msg = MSG_MON_KICK;
					break;
				}

				case RBM_CLAW:
				{
					act = "claws you.";
					do_cut = 1;
					sound_msg = MSG_MON_CLAW;
					break;
				}

				case RBM_BITE:
				{
					act = "bites you.";
					do_cut = 1;
					sound_msg = MSG_MON_BITE;
					break;
				}

				case RBM_STING:
				{
					act = "stings you.";
					sound_msg = MSG_MON_STING;
					break;
				}

				case RBM_BUTT:
				{
					act = "butts you.";
					do_stun = 1;
					sound_msg = MSG_MON_BUTT;
					break;
				}

				case RBM_CRUSH:
				{
					act = "crushes you.";
					do_stun = 1;
					sound_msg = MSG_MON_CRUSH;
					break;
				}

				case RBM_ENGULF:
				{
					act = "engulfs you.";
					sound_msg = MSG_MON_ENGULF;
					break;
				}

				case RBM_CRAWL:
				{
					act = "crawls on you.";
					sound_msg = MSG_MON_CRAWL;
					break;
				}

				case RBM_DROOL:
				{
					act = "drools on you.";
					sound_msg = MSG_MON_DROOL;
					break;
				}

				case RBM_SPIT:
				{
					act = "spits on you.";
					sound_msg = MSG_MON_SPIT;
					break;
				}

				case RBM_GAZE:
				{
					act = "gazes at you.";
					sound_msg = MSG_MON_GAZE;
					break;
				}

				case RBM_WAIL:
				{
					act = "wails at you.";
					sound_msg = MSG_MON_WAIL;
					break;
				}

				case RBM_SPORE:
				{
					act = "releases spores at you.";
					sound_msg = MSG_MON_SPORE;
					break;
				}

				case RBM_BEG:
				{
					act = "begs you for money.";
					sound_msg = MSG_MON_BEG;
					break;
				}

				case RBM_INSULT:
				{
					act = desc_insult[randint0(MAX_DESC_INSULT)];
					sound_msg = MSG_MON_INSULT;
					break;
				}

				case RBM_MOAN:
				{
					act = desc_moan[randint0(MAX_DESC_MOAN)];
					sound_msg = MSG_MON_MOAN;
					break;
				}
			}

			/* Message */
			if (act)
				msgt(sound_msg, "%^s %s", m_name, act);


			/* Hack -- assume all attacks are obvious */
			obvious = TRUE;

			/* Roll out the damage */
			if (d_dice > 0 && d_side > 0)
				damage = damroll(d_dice, d_side);
			else
				damage = 0;

			/* Apply appropriate damage */
			switch (effect)
			{
				case 0:
				{
					/* Hack -- Assume obvious */
					obvious = TRUE;

					/* Hack -- No damage */
					damage = 0;

					break;
				}

				case RBE_HURT:
				{
					/* Obvious */
					obvious = TRUE;

					/* Hack -- Player armor reduces total damage */
					damage -= (damage * ((ac < 240) ? ac : 240) / 400);

					/* Take damage */
					take_hit(p, damage, ddesc);

					break;
				}

				case RBE_POISON:
				{
					damage = adjust_dam(p, GF_POIS, damage, RANDOMISE,
						check_for_resist(p, GF_POIS, p->state.flags, TRUE));

					/* Take damage */
					take_hit(p, damage, ddesc);

					/* Take "poison" effect */
					if (player_inc_timed(p, TMD_POISONED, randint1(rlev) + 5, TRUE, TRUE))
						obvious = TRUE;

					/* Learn about the player */
					monster_learn_resists(m_ptr, p, GF_POIS);

					break;
				}

				case RBE_UN_BONUS:
				{
					/* Take damage */
					take_hit(p, damage, ddesc);

					/* Allow complete resist */
					if (!check_state(p, OF_RES_DISEN, p->state.flags))
					{
						/* Apply disenchantment */
						if (apply_disenchant(0)) obvious = TRUE;
					}

					/* Learn about the player */
					monster_learn_resists(m_ptr, p, GF_DISEN);

					break;
				}

				case RBE_UN_POWER:
				{
					int unpower = 0, newcharge;

					/* Take damage */
					take_hit(p, damage, ddesc);

					/* Find an item */
					for (k = 0; k < 10; k++)
					{
						/* Pick an item */
						i = randint0(INVEN_PACK);

						/* Obtain the item */
						o_ptr = &p->inventory[i];

						/* Skip non-objects */
						if (!o_ptr->kind) continue;

						/* Drain charged wands/staves */
						if ((o_ptr->tval == TV_STAFF) ||
						    (o_ptr->tval == TV_WAND))
						{
							/* Charged? */
							if (o_ptr->pval[DEFAULT_PVAL])
							{
								/* Get number of charge to drain */
								unpower = (rlev / (o_ptr->kind->level + 2)) + 1;

								/* Get new charge value, don't allow negative */
								newcharge = MAX((o_ptr->pval[DEFAULT_PVAL]
										- unpower),0);
								
								/* Remove the charges */
								o_ptr->pval[DEFAULT_PVAL] = newcharge;
							}
						}

						if (unpower)
						{
							int heal = rlev * unpower;

							msg("Energy drains from your pack!");

							obvious = TRUE;

							/* Don't heal more than max hp */
							heal = MIN(heal, m_ptr->maxhp - m_ptr->hp);

							/* Heal */
							m_ptr->hp += heal;

							/* Redraw (later) if needed */
							if (cave_monster(cave, p->health_who) == m_ptr)
								p->redraw |= (PR_HEALTH);

							/* Combine / Reorder the pack */
							p->notice |= (PN_COMBINE | PN_REORDER);

							/* Redraw stuff */
							p->redraw |= (PR_INVEN);

							/* Affect only a single inventory slot */
							break;
						}
					}

					break;
				}

				case RBE_EAT_GOLD:
				{
					/* Take damage */
					take_hit(p, damage, ddesc);

					/* Obvious */
					obvious = TRUE;

					/* Saving throw (unless paralyzed) based on dex and level */
					if (!p->timed[TMD_PARALYZED] &&
					    (randint0(100) < (adj_dex_safe[p->state.stat_ind[A_DEX]] +
					                      p->lev)))
					{
						/* Saving throw message */
						msg("You quickly protect your money pouch!");

						/* Occasional blink anyway */
						if (randint0(3)) blinked = TRUE;
					}

					/* Eat gold */
					else {
						gold = (p->au / 10) + randint1(25);
						if (gold < 2) gold = 2;
						if (gold > 5000) gold = (p->au / 20) + randint1(3000);
						if (gold > p->au) gold = p->au;
						p->au -= gold;
						if (gold <= 0) {
							msg("Nothing was stolen.");
							break;
						}
						/* Let the player know they were robbed */
						msg("Your purse feels lighter.");
						if (p->au)
							msg("%ld coins were stolen!", (long)gold);
						else
							msg("All of your coins were stolen!");

						/* While we have gold, put it in objects */
						while (gold > 0) {
							int amt;

							/* Create a new temporary object */
							object_type o;
							object_wipe(&o);
							object_prep(&o, objkind_get(TV_GOLD, SV_GOLD), 0, MINIMISE);

							/* Amount of gold to put in this object */
							amt = gold > MAX_PVAL ? MAX_PVAL : gold;
							o.pval[DEFAULT_PVAL] = amt;
							gold -= amt;

							/* Set origin to stolen, so it is not confused with
							 * dropped treasure in monster_death */
							o.origin = ORIGIN_STOLEN;

							/* Give the gold to the monster */
							monster_carry(m_ptr, &o);
						}

						/* Redraw gold */
						p->redraw |= (PR_GOLD);

						/* Blink away */
						blinked = TRUE;
					}

					break;
				}

				case RBE_EAT_ITEM:
				{
					/* Take damage */
					take_hit(p, damage, ddesc);

					/* Saving throw (unless paralyzed) based on dex and level */
					if (!p->timed[TMD_PARALYZED] &&
					    (randint0(100) < (adj_dex_safe[p->state.stat_ind[A_DEX]] +
					                      p->lev)))
					{
						/* Saving throw message */
						msg("You grab hold of your backpack!");

						/* Occasional "blink" anyway */
						blinked = TRUE;

						/* Obvious */
						obvious = TRUE;

						/* Done */
						break;
					}

					/* Find an item */
					for (k = 0; k < 10; k++)
					{
						object_type *i_ptr;
						object_type object_type_body;

						/* Pick an item */
						i = randint0(INVEN_PACK);

						/* Obtain the item */
						o_ptr = &p->inventory[i];

						/* Skip non-objects */
						if (!o_ptr->kind) continue;

						/* Skip artifacts */
						if (o_ptr->artifact) continue;

						/* Get a description */
						object_desc(o_name, sizeof(o_name), o_ptr, ODESC_FULL);

						/* Message */
						msg("%sour %s (%c) was stolen!",
						           ((o_ptr->number > 1) ? "One of y" : "Y"),
						           o_name, index_to_label(i));

						/* Get local object */
						i_ptr = &object_type_body;

						/* Obtain local object */
						object_copy(i_ptr, o_ptr);

						/* Modify number */
						i_ptr->number = 1;

						/* Hack -- If a rod, staff, or wand, allocate total
						 * maximum timeouts or charges between those
						 * stolen and those missed. -LM-
						 */
						distribute_charges(o_ptr, i_ptr, 1);

						/* Carry the object */
						(void)monster_carry(m_ptr, i_ptr);

						/* Steal the items */
						inven_item_increase(i, -1);
						inven_item_optimize(i);

						/* Obvious */
						obvious = TRUE;

						/* Blink away */
						blinked = TRUE;

						/* Done */
						break;
					}

					break;
				}

				case RBE_EAT_FOOD:
				{
					/* Take damage */
					take_hit(p, damage, ddesc);

					/* Steal some food */
					for (k = 0; k < 10; k++)
					{
						/* Pick an item from the pack */
						i = randint0(INVEN_PACK);

						/* Get the item */
						o_ptr = &p->inventory[i];

						/* Skip non-objects */
						if (!o_ptr->kind) continue;

						/* Skip non-food objects */
						if (o_ptr->tval != TV_FOOD) continue;

						/* Get a description */
						object_desc(o_name, sizeof(o_name), o_ptr,
									ODESC_PREFIX | ODESC_BASE);

						/* Message */
						msg("%sour %s (%c) was eaten!",
						           ((o_ptr->number > 1) ? "One of y" : "Y"),
						           o_name, index_to_label(i));

						/* Steal the items */
						inven_item_increase(i, -1);
						inven_item_optimize(i);

						/* Obvious */
						obvious = TRUE;

						/* Done */
						break;
					}

					break;
				}

				case RBE_EAT_LIGHT:
				{
					bitflag f[OF_SIZE];

					/* Take damage */
					take_hit(p, damage, ddesc);

					/* Get the light, and its flags */
					o_ptr = &p->inventory[INVEN_LIGHT];
					object_flags(o_ptr, f);

					/* Drain fuel where applicable */
					if (!of_has(f, OF_NO_FUEL) && (o_ptr->timeout > 0))
					{
						/* Reduce fuel */
						o_ptr->timeout -= (250 + randint1(250));
						if (o_ptr->timeout < 1) o_ptr->timeout = 1;

						/* Notice */
						if (!p->timed[TMD_BLIND])
						{
							msg("Your light dims.");
							obvious = TRUE;
						}

						/* Redraw stuff */
						p->redraw |= (PR_EQUIP);
					}

					break;
				}

				case RBE_ACID:
				{
					/* Obvious */
					obvious = TRUE;

					/* Message */
					msg("You are covered in acid!");

					/* Special damage */
					damage = adjust_dam(p, GF_ACID, damage, RANDOMISE, 
						check_for_resist(p, GF_ACID, p->state.flags, TRUE));
					if (damage) {
						take_hit(p, damage, ddesc);
						inven_damage(p, GF_ACID, MIN(damage * 5, 300));
					}

					/* Learn about the player */
					monster_learn_resists(m_ptr, p, GF_ACID);

					break;
				}

				case RBE_ELEC:
				{
					/* Obvious */
					obvious = TRUE;

					/* Message */
					msg("You are struck by electricity!");

					/* Take damage (special) */
					damage = adjust_dam(p, GF_ELEC, damage, RANDOMISE,
						check_for_resist(p, GF_ELEC, p->state.flags, TRUE));
					if (damage) {
						take_hit(p, damage, ddesc);
						inven_damage(p, GF_ELEC, MIN(damage * 5, 300));
					}

					/* Learn about the player */
					monster_learn_resists(m_ptr, p, GF_ELEC);

					break;
				}

				case RBE_FIRE:
				{
					/* Obvious */
					obvious = TRUE;

					/* Message */
					msg("You are enveloped in flames!");

					/* Take damage (special) */
					damage = adjust_dam(p, GF_FIRE, damage, RANDOMISE,
						check_for_resist(p, GF_FIRE, p->state.flags, TRUE));
					if (damage) {
						take_hit(p, damage, ddesc);
						inven_damage(p, GF_FIRE, MIN(damage * 5, 300));
					}

					/* Learn about the player */
					monster_learn_resists(m_ptr, p, GF_FIRE);

					break;
				}

				case RBE_COLD:
				{
					/* Obvious */
					obvious = TRUE;

					/* Message */
					msg("You are covered with frost!");

					/* Take damage (special) */
					damage = adjust_dam(p, GF_COLD, damage, RANDOMISE,
						check_for_resist(p, GF_COLD, p->state.flags, TRUE));
					if (damage) {
						take_hit(p, damage, ddesc);
						inven_damage(p, GF_COLD, MIN(damage * 5, 300));
					}

					/* Learn about the player */
					monster_learn_resists(m_ptr, p, GF_COLD);

					break;
				}

				case RBE_BLIND:
				{
					/* Take damage */
					take_hit(p, damage, ddesc);

					/* Increase "blind" */
					if (player_inc_timed(p, TMD_BLIND, 10 + randint1(rlev), TRUE, TRUE))
						obvious = TRUE;

					/* Learn about the player */
					update_smart_learn(m_ptr, p, OF_RES_BLIND);

					break;
				}

				case RBE_CONFUSE:
				{
					/* Take damage */
					take_hit(p, damage, ddesc);

					/* Increase "confused" */
					if (player_inc_timed(p, TMD_CONFUSED, 3 + randint1(rlev), TRUE, TRUE))
						obvious = TRUE;

					/* Learn about the player */
					update_smart_learn(m_ptr, p, OF_RES_CONFU);

					break;
				}

				case RBE_TERRIFY:
				{
					/* Take damage */
					take_hit(p, damage, ddesc);

					/* Increase "afraid" */
					if (randint0(100) < p->state.skills[SKILL_SAVE])
					{
						msg("You stand your ground!");
						obvious = TRUE;
					}
					else
					{
						if (player_inc_timed(p, TMD_AFRAID, 3 + randint1(rlev), TRUE,
								TRUE))
							obvious = TRUE;
					}

					/* Learn about the player */
					update_smart_learn(m_ptr, p, OF_RES_FEAR);

					break;
				}

				case RBE_PARALYZE:
				{
					/* Hack -- Prevent perma-paralysis via damage */
					if (p->timed[TMD_PARALYZED] && (damage < 1)) damage = 1;

					/* Take damage */
					take_hit(p, damage, ddesc);

					/* Increase "paralyzed" */
					if (randint0(100) < p->state.skills[SKILL_SAVE])
					{
						msg("You resist the effects!");
						obvious = TRUE;
					}
					else
					{
						if (player_inc_timed(p, TMD_PARALYZED, 3 + randint1(rlev), TRUE,
								TRUE))
							obvious = TRUE;
					}

					/* Learn about the player */
					update_smart_learn(m_ptr, p, OF_FREE_ACT);

					break;
				}

				case RBE_LOSE_STR:
				{
					/* Take damage */
					take_hit(p, damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(A_STR, FALSE)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_INT:
				{
					/* Take damage */
					take_hit(p, damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(A_INT, FALSE)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_WIS:
				{
					/* Take damage */
					take_hit(p, damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(A_WIS, FALSE)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_DEX:
				{
					/* Take damage */
					take_hit(p, damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(A_DEX, FALSE)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_CON:
				{
					/* Take damage */
					take_hit(p, damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(A_CON, FALSE)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_CHR:
				{
					/* Take damage */
					take_hit(p, damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(A_CHR, FALSE)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_ALL:
				{
					/* Take damage */
					take_hit(p, damage, ddesc);

					/* Damage (stats) */
					if (do_dec_stat(A_STR, FALSE)) obvious = TRUE;
					if (do_dec_stat(A_DEX, FALSE)) obvious = TRUE;
					if (do_dec_stat(A_CON, FALSE)) obvious = TRUE;
					if (do_dec_stat(A_INT, FALSE)) obvious = TRUE;
					if (do_dec_stat(A_WIS, FALSE)) obvious = TRUE;
					if (do_dec_stat(A_CHR, FALSE)) obvious = TRUE;

					break;
				}

				case RBE_SHATTER:
				{
					/* Obvious */
					obvious = TRUE;

					/* Hack -- Reduce damage based on the player armor class */
					damage -= (damage * ((ac < 240) ? ac : 240) / 400);

					/* Take damage */
					take_hit(p, damage, ddesc);

					/* Radius 8 earthquake centered at the monster */
					if (damage > 23)
					{
						int px_old = p->px;
						int py_old = p->py;

						earthquake(m_ptr->fy, m_ptr->fx, 8);

						/* Stop the blows if the player is pushed away */
						if ((px_old != p->px) ||
						    (py_old != p->py))
						    do_break = TRUE;
					}
					break;
				}

				case RBE_EXP_10:
				{
					/* Obvious */
					obvious = TRUE;

					/* Take damage */
					take_hit(p, damage, ddesc);
					update_smart_learn(m_ptr, p_ptr, OF_HOLD_LIFE);

					if (check_state(p, OF_HOLD_LIFE, p->state.flags) && (randint0(100) < 95))
					{
						msg("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(10, 6) + (p->exp/100) * MON_DRAIN_LIFE;
						if (check_state(p, OF_HOLD_LIFE, p->state.flags))
						{
							msg("You feel your life slipping away!");
							player_exp_lose(p, d / 10, FALSE);
						}
						else
						{
							msg("You feel your life draining away!");
							player_exp_lose(p, d, FALSE);
						}
					}

					break;
				}

				case RBE_EXP_20:
				{
					/* Obvious */
					obvious = TRUE;

					/* Take damage */
					take_hit(p, damage, ddesc);
					update_smart_learn(m_ptr, p_ptr, OF_HOLD_LIFE);

					if (check_state(p, OF_HOLD_LIFE, p->state.flags) && (randint0(100) < 90))
					{
						msg("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(20, 6) + (p->exp / 100) * MON_DRAIN_LIFE;

						if (check_state(p, OF_HOLD_LIFE, p->state.flags))
						{
							msg("You feel your life slipping away!");
							player_exp_lose(p, d / 10, FALSE);
						}
						else
						{
							msg("You feel your life draining away!");
							player_exp_lose(p, d, FALSE);
						}
					}
					break;
				}

				case RBE_EXP_40:
				{
					/* Obvious */
					obvious = TRUE;

					/* Take damage */
					take_hit(p, damage, ddesc);
					update_smart_learn(m_ptr, p_ptr, OF_HOLD_LIFE);

					if (check_state(p, OF_HOLD_LIFE, p->state.flags) && (randint0(100) < 75))
					{
						msg("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(40, 6) + (p->exp / 100) * MON_DRAIN_LIFE;

						if (check_state(p, OF_HOLD_LIFE, p->state.flags))
						{
							msg("You feel your life slipping away!");
							player_exp_lose(p, d / 10, FALSE);
						}
						else
						{
							msg("You feel your life draining away!");
							player_exp_lose(p, d, FALSE);
						}
					}
					break;
				}

				case RBE_EXP_80:
				{
					/* Obvious */
					obvious = TRUE;

					/* Take damage */
					take_hit(p, damage, ddesc);
					update_smart_learn(m_ptr, p_ptr, OF_HOLD_LIFE);

					if (check_state(p, OF_HOLD_LIFE, p->state.flags) && (randint0(100) < 50))
					{
						msg("You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(80, 6) + (p->exp / 100) * MON_DRAIN_LIFE;

						if (check_state(p, OF_HOLD_LIFE, p->state.flags))
						{
							msg("You feel your life slipping away!");
							player_exp_lose(p, d / 10, FALSE);
						}
						else
						{
							msg("You feel your life draining away!");
							player_exp_lose(p, d, FALSE);
						}
					}
					break;
				}

				case RBE_HALLU:
				{
					/* Take damage */
					take_hit(p, damage, ddesc);

					/* Increase "image" */
					if (player_inc_timed(p, TMD_IMAGE, 3 + randint1(rlev / 2), TRUE, TRUE))
						obvious = TRUE;

					/* Learn about the player */
					monster_learn_resists(m_ptr, p, GF_CHAOS);

					break;
				}
			}


			/* Hack -- only one of cut or stun */
			if (do_cut && do_stun)
			{
				/* Cancel cut */				if (randint0(100) < 50)
				{
					do_cut = 0;
				}

				/* Cancel stun */
				else
				{
					do_stun = 0;
				}
			}

			/* Handle cut */
			if (do_cut)
			{
				int k;

				/* Critical hit (zero if non-critical) */
				tmp = monster_critical(d_dice, d_side, damage);

				/* Roll for damage */
				switch (tmp)
				{
					case 0: k = 0; break;
					case 1: k = randint1(5); break;
					case 2: k = randint1(5) + 5; break;
					case 3: k = randint1(20) + 20; break;
					case 4: k = randint1(50) + 50; break;
					case 5: k = randint1(100) + 100; break;
					case 6: k = 300; break;
					default: k = 500; break;
				}

				/* Apply the cut */
				if (k) (void)player_inc_timed(p, TMD_CUT, k, TRUE, TRUE);
			}

			/* Handle stun */
			if (do_stun)
			{
				int k;

				/* Critical hit (zero if non-critical) */
				tmp = monster_critical(d_dice, d_side, damage);

				/* Roll for damage */
				switch (tmp)
				{
					case 0: k = 0; break;
					case 1: k = randint1(5); break;
					case 2: k = randint1(10) + 10; break;
					case 3: k = randint1(20) + 20; break;
					case 4: k = randint1(30) + 30; break;
					case 5: k = randint1(40) + 40; break;
					case 6: k = 100; break;
					default: k = 200; break;
				}

				/* Apply the stun */
				if (k) (void)player_inc_timed(p, TMD_STUN, k, TRUE, TRUE);
			}
		}

		/* Monster missed player */
		else
		{
			/* Analyze failed attacks */
			switch (method)
			{
				case RBM_HIT:
				case RBM_TOUCH:
				case RBM_PUNCH:
				case RBM_KICK:
				case RBM_CLAW:
				case RBM_BITE:
				case RBM_STING:
				case RBM_BUTT:
				case RBM_CRUSH:
				case RBM_ENGULF:

				/* Visible monsters */
				if (m_ptr->ml)
				{
					/* Disturbing */
					disturb(p, 1, 0);

					/* Message */
					msg("%^s misses you.", m_name);
				}

				break;
			}
		}


		/* Analyze "visible" monsters only */
		if (visible)
		{
			/* Count "obvious" attacks (and ones that cause damage) */
			if (obvious || damage || (l_ptr->blows[ap_cnt] > 10))
			{
				/* Count attacks of this type */
				if (l_ptr->blows[ap_cnt] < MAX_UCHAR)
				{
					l_ptr->blows[ap_cnt]++;
				}
			}
		}

		/* Skip the other blows if necessary */
		if (do_break) break;
	}


	/* Blink away */
	if (blinked)
	{
		msg("There is a puff of smoke!");
		teleport_away(m_ptr, MAX_SIGHT * 2 + 5);
	}


	/* Always notice cause of death */
	if (p->is_dead && (l_ptr->deaths < MAX_SHORT))
	{
		l_ptr->deaths++;
	}


	/* Assume we attacked */
	return (TRUE);
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
static void process_monster(struct cave *c, int m_idx)
{
	monster_type *m_ptr = cave_monster(cave, m_idx);
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
	if (m_ptr->m_timed[MON_TMD_SLEEP])
	{
		u32b notice;

		/* Aggravation */
		if (check_state(p_ptr, OF_AGGRAVATE, p_ptr->state.flags))
		{
			/* Wake the monster */
			mon_clear_timed(m_idx, MON_TMD_SLEEP, MON_TMD_FLG_NOTIFY);

			/* Notice the "waking up" */
			if (m_ptr->ml && !m_ptr->unaware)
			{
				char m_name[80];

				/* Get the monster name */
				monster_desc(m_name, sizeof(m_name), m_ptr, 0);

				/* Dump a message */
				msg("%^s wakes up.", m_name);

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
			if (m_ptr->m_timed[MON_TMD_SLEEP] > d)
			{
				/* Monster wakes up "a little bit" */
				mon_dec_timed(m_idx, MON_TMD_SLEEP, d , MON_TMD_FLG_NOMESSAGE);

				/* Notice the "not waking up" */
				if (m_ptr->ml && !m_ptr->unaware)
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
				woke_up = mon_clear_timed(m_idx, MON_TMD_SLEEP, MON_TMD_FLG_NOMESSAGE);

				/* Notice the "waking up" */
				if (m_ptr->ml && !m_ptr->unaware)
				{
					char m_name[80];

					/* Get the monster name */
					monster_desc(m_name, sizeof(m_name), m_ptr, 0);

					/* Dump a message */
					msg("%^s wakes up.", m_name);

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
		if (m_ptr->m_timed[MON_TMD_SLEEP]) return;
	}

	/* If the monster just woke up, then it doesn't act */
	if (woke_up) return;

	if (m_ptr->m_timed[MON_TMD_FAST])
		mon_dec_timed(m_idx, MON_TMD_FAST, 1, 0);

	if (m_ptr->m_timed[MON_TMD_SLOW])
		mon_dec_timed(m_idx, MON_TMD_SLOW, 1, 0);

	if (m_ptr->m_timed[MON_TMD_STUN])
	{
		int d = 1;

		/* Make a "saving throw" against stun */
		if (randint0(5000) <= r_ptr->level * r_ptr->level)
		{
			/* Recover fully */
			d = m_ptr->m_timed[MON_TMD_STUN];
		}

		/* Hack -- Recover from stun */
		if (m_ptr->m_timed[MON_TMD_STUN] > d)
			mon_dec_timed(m_idx, MON_TMD_STUN, 1, MON_TMD_FLG_NOMESSAGE);
		else
			mon_clear_timed(m_idx, MON_TMD_STUN, MON_TMD_FLG_NOTIFY);

		/* Still stunned */
		if (m_ptr->m_timed[MON_TMD_STUN]) return;
	}

	if (m_ptr->m_timed[MON_TMD_CONF])
	{
		int d = randint1(r_ptr->level / 10 + 1);

		/* Still confused */
		if (m_ptr->m_timed[MON_TMD_CONF] > d)
			mon_dec_timed(m_idx, MON_TMD_CONF, d , MON_TMD_FLG_NOMESSAGE);
		else
			mon_clear_timed(m_idx, MON_TMD_CONF, MON_TMD_FLG_NOTIFY);
	}

	if (m_ptr->m_timed[MON_TMD_FEAR])
	{
		/* Amount of "boldness" */
		int d = randint1(r_ptr->level / 10 + 1);

		if (m_ptr->m_timed[MON_TMD_FEAR] > d)
			mon_dec_timed(m_idx, MON_TMD_FEAR, d, MON_TMD_FLG_NOMESSAGE);
		else
			mon_clear_timed(m_idx, MON_TMD_FEAR, MON_TMD_FLG_NOTIFY);
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
				if (cave->m_idx[y][x] > 0) k++;
			}
		}

		/* Multiply slower in crowded areas */
		if ((k < 4) && (k == 0 || one_in_(k * MON_MULT_ADJ)))
		{
			/* Successful breeding attempt, learn about that now */
			if (m_ptr->ml) rf_on(l_ptr->flags, RF_MULTIPLY);
			
			/* Try to multiply (only breeders allowed) */
			if (rf_has(r_ptr->flags, RF_MULTIPLY) && multiply_monster(m_idx))
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
	if (m_ptr->m_timed[MON_TMD_CONF])
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
			if (m_ptr->ml) rf_on(l_ptr->flags, RF_RAND_25);

			/* Stagger */			
			if (flags_test(r_ptr->flags, RF_SIZE, RF_RAND_25, RF_RAND_50, FLAG_END)) stagger = TRUE;
		}
		
		/* Random movement (50%) */
		else if (roll < 50)
		{
			/* Learn about medium random movement */
			if (m_ptr->ml) rf_on(l_ptr->flags, RF_RAND_50);

			/* Stagger */			
			if (rf_has(r_ptr->flags, RF_RAND_50)) stagger = TRUE;
		}
		
		/* Random movement (75%) */
		else if (roll < 75)
		{
			/* Stagger */			
			if (flags_test_all(r_ptr->flags, RF_SIZE, RF_RAND_25, RF_RAND_50, FLAG_END))
			{
				stagger = TRUE;
			}
		}
	}

	/* Normal movement */
	if (!stagger)
	{
		/* Logical moves, may do nothing */
		if (!get_moves(cave, m_idx, mm)) return;
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
		else if (cave->feat[ny][nx] >= FEAT_PERM_EXTRA)
		{
			/* Nothing */
		}

		/* Normal wall, door, or secret door in the way */
		else
		{
			/* There's some kind of feature in the way, so learn about
			 * kill-wall and pass-wall now
			 */
			if (m_ptr->ml)
			{
				rf_on(l_ptr->flags, RF_PASS_WALL);
				rf_on(l_ptr->flags, RF_KILL_WALL);
			}

			/* Monster moves through walls (and doors) */
			if (rf_has(r_ptr->flags, RF_PASS_WALL))
			{
				/* Pass through walls/doors/rubble */
				do_move = TRUE;
			}

			/* Monster destroys walls (and doors) */
			else if (rf_has(r_ptr->flags, RF_KILL_WALL))
			{
				/* Eat through walls/doors/rubble */
				do_move = TRUE;

				/* Forget the wall */
				cave->info[ny][nx] &= ~(CAVE_MARK);

				/* Notice */
				cave_set_feat(c, ny, nx, FEAT_FLOOR);

				/* Note changes to viewable region */
				if (player_has_los_bold(ny, nx)) do_view = TRUE;
			}

			/* Handle doors and secret doors */
			else if (((cave->feat[ny][nx] >= FEAT_DOOR_HEAD) &&
						 (cave->feat[ny][nx] <= FEAT_DOOR_TAIL)) ||
						(cave->feat[ny][nx] == FEAT_SECRET))
			{
				bool may_bash = TRUE;

				/* Take a turn */
				do_turn = TRUE;
				
				/* Learn about door abilities */
				if (m_ptr->ml)
				{
					rf_on(l_ptr->flags, RF_OPEN_DOOR);
					rf_on(l_ptr->flags, RF_BASH_DOOR);
				}

				/* Creature can open doors. */
				if (rf_has(r_ptr->flags, RF_OPEN_DOOR))
				{
					/* Closed doors and secret doors */
					if ((cave->feat[ny][nx] == FEAT_DOOR_HEAD) ||
							 (cave->feat[ny][nx] == FEAT_SECRET)) {
						/* The door is open */
						did_open_door = TRUE;

						/* Do not bash the door */
						may_bash = FALSE;
					}

					/* Locked doors (not jammed) */
					else if (cave->feat[ny][nx] < FEAT_DOOR_HEAD + 0x08) {
						int k;

						/* Door power */
						k = ((cave->feat[ny][nx] - FEAT_DOOR_HEAD) & 0x07);

						/* Try to unlock it */
						if (randint0(m_ptr->hp / 10) > k) {
							msg("Something fiddles with a lock.");

							/* Reduce the power of the door by one */
							cave_set_feat(c, ny, nx, cave->feat[ny][nx] - 1);

							/* Do not bash the door */
							may_bash = FALSE;
						}
					}
				}

				/* Stuck doors -- attempt to bash them down if allowed */
				if (may_bash && rf_has(r_ptr->flags, RF_BASH_DOOR))
				{
					int k;

					/* Door power */
					k = ((cave->feat[ny][nx] - FEAT_DOOR_HEAD) & 0x07);

					/* Attempt to bash */
					if (randint0(m_ptr->hp / 10) > k) {
						msg("Something slams against a door.");

						/* Reduce the power of the door by one */
						cave_set_feat(c, ny, nx, cave->feat[ny][nx] - 1);

						/* If the door is no longer jammed */
						if (cave->feat[ny][nx] < FEAT_DOOR_HEAD + 0x09)	{
							msg("You hear a door burst open!");

							/* Disturb (sometimes) */
							disturb(p_ptr, 0, 0);

							/* The door was bashed open */
							did_bash_door = TRUE;

							/* Hack -- fall into doorway */
							do_move = TRUE;
						}
					}
				}
			}

			/* Deal with doors in the way */
			if (did_open_door || did_bash_door)
			{
				/* Break down the door */
				if (did_bash_door && (randint0(100) < 50))
				{
					cave_set_feat(c, ny, nx, FEAT_BROKEN);
				}

				/* Open the door */
				else
				{
					cave_set_feat(c, ny, nx, FEAT_OPEN);
				}

				/* Handle viewable doors */
				if (player_has_los_bold(ny, nx)) do_view = TRUE;
			}
		}


		/* Hack -- check for Glyph of Warding */
		if (do_move && (cave->feat[ny][nx] == FEAT_GLYPH))
		{
			/* Assume no move allowed */
			do_move = FALSE;

			/* Break the ward */
			if (randint1(BREAK_GLYPH) < r_ptr->level)
			{
				/* Describe observable breakage */
				if (cave->info[ny][nx] & (CAVE_MARK))
				{
					msg("The rune of protection is broken!");
				}

				/* Forget the rune */
				cave->info[ny][nx] &= ~CAVE_MARK;

				/* Break the rune */
				cave_set_feat(c, ny, nx, FEAT_FLOOR);

				/* Allow movement */
				do_move = TRUE;
			}
		}


		/* The player is in the way. */
		if (do_move && (cave->m_idx[ny][nx] < 0))
		{
			/* Learn about if the monster attacks */
			if (m_ptr->ml) rf_on(l_ptr->flags, RF_NEVER_BLOW);

			/* Some monsters never attack */
			if (rf_has(r_ptr->flags, RF_NEVER_BLOW))
			{
				/* Do not move */
				do_move = FALSE;
			}
			
			/* Otherwise, attack the player */
			else
			{
				/* Do the attack */
				make_attack_normal(m_ptr, p_ptr);

				/* Do not move */
				do_move = FALSE;

				/* Took a turn */
				do_turn = TRUE;
			}
		}


		/* Some monsters never move */
		if (do_move && rf_has(r_ptr->flags, RF_NEVER_MOVE))
		{
			/* Learn about lack of movement */
			if (m_ptr->ml) rf_on(l_ptr->flags, RF_NEVER_MOVE);

			/* Do not move */
			do_move = FALSE;
		}


		/* A monster is in the way */
		if (do_move && (cave->m_idx[ny][nx] > 0))
		{
			monster_type *n_ptr = cave_monster(cave, cave->m_idx[ny][nx]);

			/* Kill weaker monsters */
			int kill_ok = rf_has(r_ptr->flags, RF_KILL_BODY);

			/* Move weaker monsters if they can swap places */
			/* (not in a wall) */
			int move_ok = (rf_has(r_ptr->flags, RF_MOVE_BODY) &&
						   cave_floor_bold(m_ptr->fy, m_ptr->fx));

			/* Assume no movement */
			do_move = FALSE;

			if (compare_monsters(m_ptr, n_ptr) > 0)
			{
				/* Learn about pushing and shoving */
				if (m_ptr->ml)
				{
					rf_on(l_ptr->flags, RF_KILL_BODY);
					rf_on(l_ptr->flags, RF_MOVE_BODY);
				}

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
							msg("%^s tramples over %s.", m_name, n_name);
						}

						delete_monster(ny, nx);
					}
					else
					{
						/* Note if visible */
						if (m_ptr->ml && (m_ptr->mflag & (MFLAG_VIEW)))
						{
							msg("%^s pushes past %s.", m_name, n_name);
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
			if (m_ptr->ml) rf_on(l_ptr->flags, RF_NEVER_MOVE);

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
				disturb(p_ptr, 0, 0);
			}


			/* Scan all objects in the grid */
			for (this_o_idx = cave->o_idx[ny][nx]; this_o_idx; this_o_idx = next_o_idx)
			{
				object_type *o_ptr;

				/* Get the object */
				o_ptr = object_byid(this_o_idx);

				/* Get the next object */
				next_o_idx = o_ptr->next_o_idx;

				/* Skip gold */
				if (o_ptr->tval == TV_GOLD) continue;
				
				/* Learn about item pickup behavior */
				if (m_ptr->ml)
				{
					rf_on(l_ptr->flags, RF_TAKE_ITEM);
					rf_on(l_ptr->flags, RF_KILL_ITEM);
				}

				/* Take or Kill objects on the floor */
				if (rf_has(r_ptr->flags, RF_TAKE_ITEM) || rf_has(r_ptr->flags, RF_KILL_ITEM))
				{
					bitflag obj_flags[OF_SIZE];
					bitflag mon_flags[RF_SIZE];

					char m_name[80];
					char o_name[80];

					rf_wipe(mon_flags);

					/* Extract some flags */
					object_flags(o_ptr, obj_flags);

					/* Get the object name */
					object_desc(o_name, sizeof(o_name), o_ptr,
								ODESC_PREFIX | ODESC_FULL);

					/* Get the monster name */
					monster_desc(m_name, sizeof(m_name), m_ptr, MDESC_IND1);

					/* React to objects that hurt the monster */
					react_to_slay(obj_flags, mon_flags);

					/* The object cannot be picked up by the monster */
					if (o_ptr->artifact || rf_is_inter(r_ptr->flags, mon_flags))
					{
						/* Only give a message for "take_item" */
						if (rf_has(r_ptr->flags, RF_TAKE_ITEM))
						{
							/* Describe observable situations */
							if (m_ptr->ml && player_has_los_bold(ny, nx) && !squelch_item_ok(o_ptr))
							{
								/* Dump a message */
								msg("%^s tries to pick up %s, but fails.",
								           m_name, o_name);
							}
						}
					}

					/* Pick up the item */
					else if (rf_has(r_ptr->flags, RF_TAKE_ITEM))
					{
						object_type *i_ptr;
						object_type object_type_body;

						/* Describe observable situations */
						if (player_has_los_bold(ny, nx) && !squelch_item_ok(o_ptr))
						{
							/* Dump a message */
							msg("%^s picks up %s.", m_name, o_name);
						}

						/* Get local object */
						i_ptr = &object_type_body;

						/* Obtain local object */
						object_copy(i_ptr, o_ptr);

						/* Delete the object */
						delete_object_idx(this_o_idx);

						/* Carry the object */
						monster_carry(m_ptr, i_ptr);
					}

					/* Destroy the item */
					else
					{
						/* Describe observable situations */
						if (player_has_los_bold(ny, nx) && !squelch_item_ok(o_ptr))
						{
							/* Dump a message */
							msgt(MSG_DESTROY, "%^s crushes %s.", m_name, o_name);
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
	if (OPT(birth_ai_smart) && !do_turn && !do_move)
	{
		/* Cast spell */
		if (make_attack_spell(m_idx)) return;
	}

	if (rf_has(r_ptr->flags, RF_HAS_LIGHT)) do_view = TRUE;

	/* Notice changes in view */
	if (do_view)
	{
		/* Update the visuals */
		p_ptr->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

		/* Fully update the flow XXX XXX XXX */
		p_ptr->update |= (PU_FORGET_FLOW | PU_UPDATE_FLOW);
	}


	/* Hack -- get "bold" if out of options */
	if (!do_turn && !do_move && m_ptr->m_timed[MON_TMD_FEAR])
	{
		mon_clear_timed(m_idx, MON_TMD_FEAR, MON_TMD_FLG_NOTIFY);
	}

	/* If we see an unaware monster do something, become aware of it */
	if (do_turn && m_ptr->unaware)
	{
		m_ptr->unaware = FALSE;

		/* Learn about mimicry */
		if (rf_has(r_ptr->flags, RF_UNAWARE))
			rf_on(l_ptr->flags, RF_UNAWARE);
	}

}


static bool monster_can_flow(struct cave *c, int m_idx)
{
	monster_type *m_ptr;
	monster_race *r_ptr;
	int fy, fx;

	assert(c);

	m_ptr = cave_monster(cave, m_idx);
	r_ptr = &r_info[m_ptr->r_idx];
	fy = m_ptr->fy;
	fx = m_ptr->fx;

	/* Check the flow (normal aaf is about 20) */
	if ((c->when[fy][fx] == c->when[p_ptr->py][p_ptr->px]) &&
	    (c->cost[fy][fx] < MONSTER_FLOW_DEPTH) &&
	    (c->cost[fy][fx] < r_ptr->aaf))
		return TRUE;
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
 * Most of the rest of the time is spent in "update_view()" and "light_spot()",
 * especially when the player is running.
 *
 * Note the special "MFLAG_NICE" flag, which prevents "nasty" monsters from
 * using any of their spell attacks until the player gets a turn.
 */
void process_monsters(struct cave *c, byte minimum_energy)
{
	int i;

	monster_type *m_ptr;
	monster_race *r_ptr;

	/* Process the monsters (backwards) */
	for (i = cave_monster_max(c) - 1; i >= 1; i--)
	{
		/* Handle "leaving" */
		if (p_ptr->leaving) break;


		/* Get the monster */
		m_ptr = cave_monster(cave, i);


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
		    monster_can_flow(c, i))
		{
			/* Process the monster */
			process_monster(c, i);
		}
	}
}

/* Test functions */
bool (*testfn_make_attack_normal)(struct monster *m, struct player *p) = make_attack_normal;
