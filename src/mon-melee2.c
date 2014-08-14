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
#include "monster.h"
#include "mon-make.h"
#include "mon-spell.h"
#include "mon-timed.h"
#include "mon-util.h"
#include "mon-blow-methods.h"
#include "mon-blow-effects.h"
#include "obj-desc.h"
#include "obj-identify.h"
#include "obj-ignore.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"
#include "spells.h"
#include "tables.h"

/*
 * Determine if a bolt will arrive, checking that no monsters are in the way
 */
#define clean_shot(C, Y1, X1, Y2, X2)				\
	projectable(C, Y1, X1, Y2, X2, PROJECT_STOP)

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
static void remove_bad_spells(struct monster *m_ptr, bitflag f[RSF_SIZE])
{
	bitflag f2[RSF_SIZE], ai_flags[OF_SIZE];
	struct element_info el[ELEM_MAX];

	u32b smart = 0L;
	bool know_something = FALSE;

	/* Stupid monsters act randomly */
	if (rf_has(m_ptr->race->flags, RF_STUPID)) return;

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
		size_t i;

		/* Occasionally forget player status */
		if (one_in_(100)) {
			of_wipe(m_ptr->known_pstate.flags);
			for (i = 0; i < ELEM_MAX; i++)
				m_ptr->known_pstate.el_info[i].res_level = 0;
		}

		/* Use the memorized info */
		smart = m_ptr->smart;
		of_copy(ai_flags, m_ptr->known_pstate.flags);
		if (!of_is_empty(ai_flags))
			know_something = TRUE;

		for (i = 0; i < ELEM_MAX; i++) {
			el[i].res_level = m_ptr->known_pstate.el_info[i].res_level;
			if (el[i].res_level != 0)
				know_something = TRUE;
		}
	}

	/* Cancel out certain flags based on knowledge */
	if (know_something)
		unset_spells(f2, ai_flags, el, m_ptr->race);

	if (smart & SM_IMM_MANA && randint0(100) <
			50 * (rf_has(m_ptr->race->flags, RF_SMART) ? 2 : 1))
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
			if (!square_in_bounds(cave, y, x)) continue;

			/* Only check a circular area */
			if (distance(y1, x1, y, x) > 2) continue;

			/* Hack: no summon on glyph of warding */
			if (square_iswarded(cave, y, x)) continue;

			/* Require empty floor grid in line of sight */
			if (square_isempty(cave, y, x) && los(cave, y1, x1, y, x))
			{
				return (TRUE);
			}
		}
	}

	return FALSE;
}


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
static int choose_attack_spell(struct monster *m_ptr, bitflag f[RSF_SIZE])
{
	int num = 0;
	byte spells[RSF_MAX];

	int i;

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
bool make_attack_spell(struct monster *m_ptr)
{
	int chance, thrown_spell, rlev, failrate;

	bitflag f[RSF_SIZE];

	monster_lore *l_ptr = get_lore(m_ptr->race);

	char m_name[80], m_poss[80], ddesc[80];

	/* Player position */
	int px = player->px;
	int py = player->py;

	/* Extract the blind-ness */
	bool blind = (player->timed[TMD_BLIND] ? TRUE : FALSE);

	/* Extract the "see-able-ness" */
	bool seen = (!blind && m_ptr->ml);

	/* Assume "normal" target */
	bool normal = TRUE;

	/* Handle "leaving" */
	if (player->upkeep->leaving) return FALSE;

	/* Cannot cast spells when confused */
	if (m_ptr->m_timed[MON_TMD_CONF]) return (FALSE);

	/* Cannot cast spells when nice */
	if (m_ptr->mflag & MFLAG_NICE) return FALSE;

	/* Hack -- Extract the spell probability */
	chance = (m_ptr->race->freq_innate + m_ptr->race->freq_spell) / 2;

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
		if (!projectable(cave, m_ptr->fy, m_ptr->fx, py, px, PROJECT_NONE))
			return FALSE;
	}

	/* Extract the monster level */
	rlev = ((m_ptr->race->level >= 1) ? m_ptr->race->level : 1);

	/* Extract the racial spell flags */
	rsf_copy(f, m_ptr->race->spell_flags);

	/* Allow "desperate" spells */
	if (rf_has(m_ptr->race->flags, RF_SMART) &&
	    m_ptr->hp < m_ptr->maxhp / 10 &&
	    randint0(100) < 50)

		/* Require intelligent spells */
		set_spells(f, RST_HASTE | RST_ANNOY | RST_ESCAPE | RST_HEAL | RST_TACTIC | RST_SUMMON);

	/* Remove the "ineffective" spells */
	remove_bad_spells(m_ptr, f);

	/* Check whether summons and bolts are worth it. */
	if (!rf_has(m_ptr->race->flags, RF_STUPID))
	{
		/* Check for a clean bolt shot */
		if (test_spells(f, RST_BOLT) &&
			!clean_shot(cave, m_ptr->fy, m_ptr->fx, py, px))

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
	monster_desc(m_name, sizeof(m_name), m_ptr, MDESC_STANDARD);

	/* Get the monster possessive ("his"/"her"/"its") */
	monster_desc(m_poss, sizeof(m_poss), m_ptr, MDESC_PRO_VIS | MDESC_POSS);

	/* Get the "died from" name */
	monster_desc(ddesc, sizeof(ddesc), m_ptr, MDESC_DIED_FROM);

	/* Choose a spell to cast */
	thrown_spell = choose_attack_spell(m_ptr, f);

	/* Abort if no spell was chosen */
	if (!thrown_spell) return FALSE;

	/* If we see an unaware monster try to cast a spell, become aware of it */
	if (m_ptr->unaware)
		become_aware(m_ptr);

	/* Calculate spell failure rate */
	failrate = 25 - (rlev + 3) / 4;
	if (m_ptr->m_timed[MON_TMD_FEAR])
		failrate += 20;

	/* Stupid monsters will never fail (for jellies and such) */
	if (rf_has(m_ptr->race->flags, RF_STUPID))
		failrate = 0;

	/* Check for spell failure (innate attacks never fail) */
	if ((thrown_spell >= MIN_NONINNATE_SPELL) && (randint0(100) < failrate))
	{
		/* Message */
		msg("%s tries to cast a spell, but fails.", m_name);

		return TRUE;
	}

	/* Cast the spell. */
	disturb(player, 1);

	/* Special case RSF_HASTE until TMD_* and MON_TMD_* are rationalised */
	if (thrown_spell == RSF_HASTE) {
		if (blind)
			msg("%s mumbles.", m_name);
		else
			msg("%s concentrates on %s body.", m_name, m_poss);

		(void)mon_inc_timed(m_ptr, MON_TMD_FAST, 50, 0, FALSE);
	} else {
		do_mon_spell(thrown_spell, m_ptr, seen);
	}

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
	if (player->is_dead && (l_ptr->deaths < MAX_SHORT)) {
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
static int mon_will_run(struct monster *m_ptr)
{
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
	p_lev = player->lev;

	/* Examine monster power (level plus morale) */
	m_lev = m_ptr->race->level + (m_ptr->midx & 0x08) + 25;

	/* Optimize extreme cases below */
	if (m_lev > p_lev + 4) return (FALSE);
	if (m_lev + 4 <= p_lev) return (TRUE);

	/* Examine player health */
	p_chp = player->chp;
	p_mhp = player->mhp;

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

/* From Will Asher in DJA:
 * Find whether a monster is near a permanent wall
 * this decides whether PASS_WALL & KILL_WALL monsters 
 * use the monster flow code
 */
static bool near_permwall(const monster_type *m_ptr, struct chunk *c)
{
	int y, x;
	int my = m_ptr->fy;
	int mx = m_ptr->fx;
	
	/* if PC is in LOS, there's no need to go around walls */
    if (projectable(cave, my, mx, player->py, player->px, PROJECT_NONE)) 
		return FALSE;
    
    /* PASS_WALL & KILL_WALL monsters occasionally flow for a turn anyway */
    if (randint0(99) < 5) return TRUE;
    
	/* Search the nearby grids, which are always in bounds */
	for (y = (my - 2); y <= (my + 2); y++)
	{
		for (x = (mx - 2); x <= (mx + 2); x++)
		{
            if (!square_in_bounds_fully(c, y, x)) continue;
            if (square_isperm(c, y, x)) return TRUE;
		}
	}
	return FALSE;
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
static bool get_moves_aux(struct chunk *c, struct monster *m_ptr, int *yp, int *xp)
{
	int py = player->py;
	int px = player->px;

	int i, y, x, y1, x1;

	int when = 0;
	int cost = 999;

	/* Monster can go through rocks */
	if (flags_test(m_ptr->race->flags, RF_SIZE, RF_PASS_WALL, RF_KILL_WALL, FLAG_END)){
	
	    /* If monster is near a permwall, use normal pathfinding */
	    if (!near_permwall(m_ptr, c)) return (FALSE);
    }
		
	/* Monster location */
	y1 = m_ptr->fy;
	x1 = m_ptr->fx;

	/* The player is not currently near the monster grid */
	if (c->when[y1][x1] < c->when[py][px])
	{
		/* The player has never been near the monster grid */
		if (c->when[y1][x1] == 0) return (FALSE);
	}

	/* Monster is too far away to notice the player */
	if (c->cost[y1][x1] > MONSTER_FLOW_DEPTH) return (FALSE);
	if (c->cost[y1][x1] > (OPT(birth_small_range) ? m_ptr->race->aaf / 2 : m_ptr->race->aaf)) return (FALSE);

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
static bool get_fear_moves_aux(struct chunk *c, struct monster *m_ptr, int *yp, int *xp)
{
	int y, x, y1, x1, fy, fx, py, px, gy = 0, gx = 0;
	int when = 0, score = -1;
	int i;

	/* Player location */
	py = player->py;
	px = player->px;

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
	if (c->cost[fy][fx] > (OPT(birth_small_range) ? m_ptr->race->aaf / 2 : m_ptr->race->aaf)) return (FALSE);

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
static bool find_safety(struct chunk *c, struct monster *m_ptr, int *yp, int *xp)
{
	int fy = m_ptr->fy;
	int fx = m_ptr->fx;

	int py = player->py;
	int px = player->px;

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
			if (!square_in_bounds_fully(cave, y, x)) continue;

			/* Skip locations in a wall */
			if (!square_ispassable(cave, y, x)) continue;

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
static bool find_hiding(struct monster *m_ptr, int *yp, int *xp)
{
	int fy = m_ptr->fy;
	int fx = m_ptr->fx;

	int py = player->py;
	int px = player->px;

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
			if (!square_in_bounds_fully(cave, y, x)) continue;

			/* Skip occupied locations */
			if (!square_isempty(cave, y, x)) continue;

			/* Check for hidden, available grid */
			if (!player_has_los_bold(y, x) && (clean_shot(cave, fy, fx, y, x)))
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
static bool get_moves(struct chunk *c, struct monster *m_ptr, int mm[5])
{
	int py = player->py;
	int px = player->px;

	int y, ay, x, ax;

	int move_val = 0;

	int y2 = py;
	int x2 = px;

	bool done = FALSE;

	/* Flow towards the player */
	get_moves_aux(c, m_ptr, &y2, &x2);

	/* Extract the "pseudo-direction" */
	y = m_ptr->fy - y2;
	x = m_ptr->fx - x2;



	/* Normal animal packs try to get the player out of corridors. */
	if (rf_has(m_ptr->race->flags, RF_GROUP_AI) &&
	    !flags_test(m_ptr->race->flags, RF_SIZE, RF_PASS_WALL, RF_KILL_WALL, FLAG_END))
	{
		int i, open = 0;

		/* Count empty grids next to player */
		for (i = 0; i < 8; i++)
		{
			int ry = py + ddy_ddd[i];
			int rx = px + ddx_ddd[i];
			/* Check grid around the player for room interior (room walls count)
			   or other empty space */
			if (square_ispassable(cave, ry, rx) || square_isroom(cave, ry, rx))
			{
				/* One more open grid */
				open++;
			}
		}

		/* Not in an empty space and strong player */
		if ((open < 7) && (player->chp > player->mhp / 2))
		{
			/* Find hiding place */
			if (find_hiding(m_ptr, &y, &x)) done = TRUE;
		}
	}


	/* Apply fear */
	if (!done && mon_will_run(m_ptr))
	{
		/* Try to find safe place */
		if (!find_safety(c, m_ptr, &y, &x))
		{
			/* This is not a very "smart" method XXX XXX */
			y = (-y);
			x = (-x);
		}

		else
		{
			/* Adjust movement */
			get_fear_moves_aux(c, m_ptr, &y, &x);
		}

		done = TRUE;
	}


	/* Monster groups try to surround the player */
	if (!done && rf_has(m_ptr->race->flags, RF_GROUP_AI))
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
				if (!square_isempty(cave, y2, x2)) continue;
				
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
static int compare_monsters(const struct monster *m_ptr, const struct monster *n_ptr)
{
	u32b mexp1 = m_ptr->race->mexp;
	u32b mexp2 = n_ptr->race->mexp;

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

/*
 * Calculate how much damage remains after armor is taken into account
 * (does for a physical attack what adjust_dam does for an elemental attack).
 */
int adjust_dam_armor(int damage, int ac)
{
	return damage - (damage * ((ac < 240) ? ac : 240) / 400);
}

/*
 * Attack the player via physical attacks.
 */
static bool make_attack_normal(struct monster *m_ptr, struct player *p)
{
	monster_lore *l_ptr = get_lore(m_ptr->race);
	int ap_cnt;
	int k, tmp, ac, rlev;
	char m_name[80];
	char ddesc[80];
	bool blinked;

	/* Not allowed to attack */
	if (rf_has(m_ptr->race->flags, RF_NEVER_BLOW)) return (FALSE);

	/* Total armor */
	ac = p->state.ac + p->state.to_a;

	/* Extract the effective monster level */
	rlev = ((m_ptr->race->level >= 1) ? m_ptr->race->level : 1);


	/* Get the monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), m_ptr, MDESC_STANDARD);

	/* Get the "died from" information (i.e. "a kobold") */
	monster_desc(ddesc, sizeof(ddesc), m_ptr, MDESC_SHOW | MDESC_IND_VIS);

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
		int do_cut = 0;
		int do_stun = 0;
		int sound_msg = MSG_GENERIC;

		const char *act = NULL;

		/* Extract the attack infomation */
		int effect = m_ptr->race->blow[ap_cnt].effect;
		int method = m_ptr->race->blow[ap_cnt].method;
		int d_dice = m_ptr->race->blow[ap_cnt].d_dice;
		int d_side = m_ptr->race->blow[ap_cnt].d_side;

		/* Hack -- no more attacks */
		if (!method) break;

		/* Handle "leaving" */
		if (p->upkeep->leaving) break;

		/* Extract visibility (before blink) */
		if (m_ptr->ml) visible = TRUE;

		/* Extract visibility from carrying light */
		if (rf_has(m_ptr->race->flags, RF_HAS_LIGHT)) visible = TRUE;

		/* Extract the attack "power" */
		power = monster_blow_effect_power(effect);

		/* Monster hits player */
		if (!effect || check_hit(p, power, rlev)) {
			melee_effect_handler_f effect_handler;

			/* Always disturbing */
			disturb(p, 1);

			/* Hack -- Apply "protection from evil" */
			if (p->timed[TMD_PROTEVIL] > 0) {
				/* Learn about the evil flag */
				if (m_ptr->ml)
					rf_on(l_ptr->flags, RF_EVIL);

				if (rf_has(m_ptr->race->flags, RF_EVIL) && p->lev >= rlev &&
				    randint0(100) + p->lev > 50) {
					/* Message */
					msg("%s is repelled.", m_name);

					/* Hack -- Next attack */
					continue;
				}
			}

			/* Describe the attack method */
			act = monster_blow_method_action(method);
			do_cut = monster_blow_method_cut(method);
			do_stun = monster_blow_method_stun(method);
			sound_msg = monster_blow_method_message(method);

			/* Message */
			if (act)
				msgt(sound_msg, "%s %s", m_name, act);

			/* Hack -- assume all attacks are obvious */
			obvious = TRUE;

			/* Roll out the damage */
			if (d_dice > 0 && d_side > 0)
				damage = damroll(d_dice, d_side);
			else
				damage = 0;

			/* Perform the actual effect. */
			effect_handler = melee_handler_for_blow_effect(effect);

			if (effect_handler != NULL) {
				melee_effect_handler_context_t context = {
					p,
					m_ptr,
					rlev,
					method,
					ac,
					ddesc,
					obvious,
					blinked,
					do_break,
					damage,
				};

				effect_handler(&context);

				/* Save any changes made in the handler for later use. */
				obvious = context.obvious;
				blinked = context.blinked;
				damage = context.damage;
			} else
				bell(format("Effect handler not found for %d.", effect));


			/* Hack -- only one of cut or stun */
			if (do_cut && do_stun) {
				/* Cancel cut */
				if (randint0(100) < 50)
					do_cut = 0;

				/* Cancel stun */
				else
					do_stun = 0;
			}

			/* Handle cut */
			if (do_cut) {
				/* Critical hit (zero if non-critical) */
				tmp = monster_critical(d_dice, d_side, damage);

				/* Roll for damage */
				switch (tmp) {
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
			if (do_stun) {
				/* Critical hit (zero if non-critical) */
				tmp = monster_critical(d_dice, d_side, damage);

				/* Roll for damage */
				switch (tmp) {
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
				if (k)
					(void)player_inc_timed(p, TMD_STUN, k, TRUE, TRUE);
			}
		} else {
			/* Visible monster missed player, so notify if appropriate. */
			if (m_ptr->ml && monster_blow_method_miss(method)) {
				/* Disturbing */
				disturb(p, 1);
				msg("%s misses you.", m_name);
			}
		}

		/* Analyze "visible" monsters only */
		if (visible) {
			/* Count "obvious" attacks (and ones that cause damage) */
			if (obvious || damage || (l_ptr->blows[ap_cnt] > 10)) {
				/* Count attacks of this type */
				if (l_ptr->blows[ap_cnt] < MAX_UCHAR)
					l_ptr->blows[ap_cnt]++;
			}
		}

		/* Skip the other blows if necessary */
		if (do_break) break;
	}

	/* Blink away */
	if (blinked) {
		msg("There is a puff of smoke!");
		teleport_away(m_ptr, MAX_SIGHT * 2 + 5);
	}

	/* Always notice cause of death */
	if (p->is_dead && (l_ptr->deaths < MAX_SHORT))
		l_ptr->deaths++;

	/* Assume we attacked */
	return (TRUE);
}

/**
 * Process a monster's timed effects, e.g. decrease them.
 *
 * Returns TRUE if the monster is skipping its turn.
 */
static bool process_monster_timed(struct chunk *c, struct monster *m_ptr, const char *m_name)
{
	monster_lore *l_ptr = get_lore(m_ptr->race);

	/* Handle "sleep" */
	if (m_ptr->m_timed[MON_TMD_SLEEP]) {
		bool woke_up = FALSE;

		/* Anti-stealth */
		int notice = randint0(1024);

		/* Aggravation */
		if (player_of_has(player, OF_AGGRAVATE)) {
			/* Wake the monster and notify player */
			mon_clear_timed(m_ptr, MON_TMD_SLEEP, MON_TMD_FLG_NOTIFY, FALSE);

			/* Update the health bar */
			if (m_ptr->ml && !m_ptr->unaware) {
				/* Hack -- Update the health bar */
				if (player->upkeep->health_who == m_ptr)
					player->upkeep->redraw |= (PR_HEALTH);
			}

			woke_up = TRUE;

		/* Hack See if monster "notices" player */
		} else if ((notice * notice * notice) <= player->state.noise) {
			int d = 1;

			/* Wake up faster near the player */
			if (m_ptr->cdis < 50) d = (100 / m_ptr->cdis);

			/* Still asleep */
			if (m_ptr->m_timed[MON_TMD_SLEEP] > d) {
				/* Monster wakes up "a little bit" */
				mon_dec_timed(m_ptr, MON_TMD_SLEEP, d, MON_TMD_FLG_NOMESSAGE,
					FALSE);

				/* Notice the "not waking up" */
				if (m_ptr->ml && !m_ptr->unaware) {
					/* Hack -- Count the ignores */
					if (l_ptr->ignore < MAX_UCHAR)
						l_ptr->ignore++;
				}
			} else {
				/* Reset sleep counter */
				woke_up = mon_clear_timed(m_ptr, MON_TMD_SLEEP,
					MON_TMD_FLG_NOMESSAGE, FALSE);

				/* Notice the "waking up" */
				if (m_ptr->ml && !m_ptr->unaware) {
					/* Dump a message */
					msg("%s wakes up.", m_name);

					/* Hack -- Update the health bar */
					if (player->upkeep->health_who == m_ptr)
						player->upkeep->redraw |= (PR_HEALTH);

					/* Hack -- Count the wakings */
					if (l_ptr->wake < MAX_UCHAR)
						l_ptr->wake++;
				}
			}
		}

		/* Sleeping monsters don't recover in any other ways */
		/* If the monster just woke up, then it doesn't act */
		if (m_ptr->m_timed[MON_TMD_SLEEP] || woke_up) return TRUE;
	}

	if (m_ptr->m_timed[MON_TMD_FAST])
		mon_dec_timed(m_ptr, MON_TMD_FAST, 1, 0, FALSE);

	if (m_ptr->m_timed[MON_TMD_SLOW])
		mon_dec_timed(m_ptr, MON_TMD_SLOW, 1, 0, FALSE);

	if (m_ptr->m_timed[MON_TMD_STUN]) {
		int d = 1;

		/* Make a "saving throw" against stun */
		if (randint0(5000) <= m_ptr->race->level * m_ptr->race->level)
			/* Recover fully */
			d = m_ptr->m_timed[MON_TMD_STUN];

		/* Hack -- Recover from stun */
		if (m_ptr->m_timed[MON_TMD_STUN] > d)
			mon_dec_timed(m_ptr, MON_TMD_STUN, 1, MON_TMD_FLG_NOMESSAGE, FALSE);
		else
			mon_clear_timed(m_ptr, MON_TMD_STUN, MON_TMD_FLG_NOTIFY, FALSE);
	}

	if (m_ptr->m_timed[MON_TMD_CONF]) {
		int d = randint1(m_ptr->race->level / 10 + 1);

		/* Still confused */
		if (m_ptr->m_timed[MON_TMD_CONF] > d)
			mon_dec_timed(m_ptr, MON_TMD_CONF, d, MON_TMD_FLG_NOMESSAGE, FALSE);
		else
			mon_clear_timed(m_ptr, MON_TMD_CONF, MON_TMD_FLG_NOTIFY, FALSE);
	}

	if (m_ptr->m_timed[MON_TMD_FEAR]) {
		/* Amount of "boldness" */
		int d = randint1(m_ptr->race->level / 10 + 1);

		if (m_ptr->m_timed[MON_TMD_FEAR] > d)
			mon_dec_timed(m_ptr, MON_TMD_FEAR, d, MON_TMD_FLG_NOMESSAGE, FALSE);
		else
			mon_clear_timed(m_ptr, MON_TMD_FEAR, MON_TMD_FLG_NOTIFY, FALSE);
	}

	/* Don't do anything if stunned */
	return m_ptr->m_timed[MON_TMD_STUN] ? TRUE : FALSE;
}


/** 
 * Attempt to reproduce, if possible.  Should only be passed monsters who are able
 * to reproduce.
 */
static bool process_monster_multiply(struct chunk *c, struct monster *m_ptr)
{
	int oy = m_ptr->fy;
	int ox = m_ptr->fx;

	int k = 0, y, x;

	monster_lore *l_ptr = get_lore(m_ptr->race);

	assert(rf_has(m_ptr->race->flags, RF_MULTIPLY));

	/* Attempt to "mutiply" (all monsters are allowed an attempt for lore
	 * purposes, even non-breeders) */
	if (num_repro >= MAX_REPRO) return FALSE;

	/* Count the adjacent monsters */
	for (y = oy - 1; y <= m_ptr->fy + 1; y++)
		for (x = ox - 1; x <= m_ptr->fx + 1; x++)
			/* Count monsters */
			if (c->m_idx[y][x] > 0) k++;

	/* Multiply slower in crowded areas */
	if ((k < 4) && (k == 0 || one_in_(k * MON_MULT_ADJ))) {
		/* Successful breeding attempt, learn about that now */
		if (m_ptr->ml)
			rf_on(l_ptr->flags, RF_MULTIPLY);

		/* Try to multiply */
		if (multiply_monster(m_ptr)) {
			/* Make a sound */
			if (m_ptr->ml)
				sound(MSG_MULTIPLY);

			/* Multiplying takes energy */
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Check if a monster should stagger or not.  Always stagger when confused,
 * but also deal with random movement for RAND_25 and _50 monsters.
 */
static bool process_monster_should_stagger(struct monster *m_ptr)
{
	monster_lore *l_ptr = get_lore(m_ptr->race);

	/* Random movement - always attempt for lore purposes */
	int roll = randint0(100);

	/* Confused */
	if (m_ptr->m_timed[MON_TMD_CONF])
		return TRUE;

	/* Random movement (25%) */
	if (roll < 25) {
		/* Learn about small random movement */
		if (m_ptr->ml)
			rf_on(l_ptr->flags, RF_RAND_25);

		/* Stagger */
		if (flags_test(m_ptr->race->flags, RF_SIZE, RF_RAND_25, RF_RAND_50, FLAG_END))
			return TRUE;

	/* Random movement (50%) */
	} else if (roll < 50) {
		/* Learn about medium random movement */
		if (m_ptr->ml)
			rf_on(l_ptr->flags, RF_RAND_50);

		/* Stagger */
		if (rf_has(m_ptr->race->flags, RF_RAND_50))
			return TRUE;

	/* Random movement (75%) */
	} else if (roll < 75) {
		/* Stagger */
		if (flags_test_all(m_ptr->race->flags, RF_SIZE, RF_RAND_25, RF_RAND_50, FLAG_END))
			return TRUE;
	}

	return FALSE;
}

/**
 * Work out if a monster can move through the grid, if necessary bashing 
 * down doors in the way.
 *
 * Returns TRUE if the monster is able to move through the grid.
 */
static bool process_monster_can_move(struct chunk *c, struct monster *m_ptr,
		const char *m_name, int nx, int ny, bool *do_view, bool *do_turn)
{
	monster_lore *l_ptr = get_lore(m_ptr->race);

	/* Floor is open? */
	if (square_ispassable(c, ny, nx))
		return TRUE;

	/* Permanent wall in the way */
	if (square_iswall(c, ny, nx) && square_isperm(c, ny, nx))
		return FALSE;

	/* Normal wall, door, or secret door in the way */

	/* There's some kind of feature in the way, so learn about
	 * kill-wall and pass-wall now */
	if (m_ptr->ml) {
		rf_on(l_ptr->flags, RF_PASS_WALL);
		rf_on(l_ptr->flags, RF_KILL_WALL);
	}

	/* Monster moves through walls (and doors) */
	if (rf_has(m_ptr->race->flags, RF_PASS_WALL)) 
		return TRUE;

	/* Monster destroys walls (and doors) */
	else if (rf_has(m_ptr->race->flags, RF_KILL_WALL)) {
		/* Forget the wall */
		sqinfo_off(c->info[ny][nx], SQUARE_MARK);

		/* Notice */
		square_destroy_wall(c, ny, nx);

		/* Note changes to viewable region */
		if (player_has_los_bold(ny, nx)) *do_view = TRUE;

		return TRUE;
	}

	/* Handle doors and secret doors */
	else if (square_iscloseddoor(c, ny, nx) || square_issecretdoor(c, ny, nx)) {
		bool may_bash = rf_has(m_ptr->race->flags, RF_BASH_DOOR) && one_in_(2);

		/* Take a turn */
		*do_turn = TRUE;

		/* Learn about door abilities */
		if (m_ptr->ml) {
			rf_on(l_ptr->flags, RF_OPEN_DOOR);
			rf_on(l_ptr->flags, RF_BASH_DOOR);
		}

		/* Creature can open or bash doors */
		if (!rf_has(m_ptr->race->flags, RF_OPEN_DOOR) && !rf_has(m_ptr->race->flags, RF_BASH_DOOR))
			return FALSE;

		/* Stuck door -- try to unlock it */
		if (square_islockeddoor(c, ny, nx)) {
			int k = square_door_power(c, ny, nx);

			if (randint0(m_ptr->hp / 10) > k) {
				if (may_bash)
					msg("%s slams against the door.", m_name);
				else
					msg("%s fiddles with the lock.", m_name);

				/* Reduce the power of the door by one */
				square_set_feat(c, ny, nx, c->feat[ny][nx] - 1);
			}
		} else {
			/* Handle viewable doors */
			if (player_has_los_bold(ny, nx))
				*do_view = TRUE;

			/* Closed or secret door -- open or bash if allowed */
			if (may_bash) {
				square_smash_door(c, ny, nx);

				msg("You hear a door burst open!");
				disturb(player, 0);

				/* Fall into doorway */
				return TRUE;
			} else if (rf_has(m_ptr->race->flags, RF_OPEN_DOOR)) {
				square_open_door(c, ny, nx);
			}
		}
	}

	return FALSE;
}

/**
 * Try to break a glyph.
 */
static bool process_monster_glyph(struct chunk *c, struct monster *m_ptr, int nx, int ny)
{
	assert(square_iswarded(c, ny, nx));

	/* Break the ward */
	if (randint1(BREAK_GLYPH) < m_ptr->race->level) {
		/* Describe observable breakage */
		if (sqinfo_has(c->info[ny][nx], SQUARE_MARK))
			msg("The rune of protection is broken!");

		/* Forget the rune */
		sqinfo_off(c->info[ny][nx], SQUARE_MARK);

		/* Break the rune */
		square_remove_ward(c, ny, nx);

		return TRUE;
	}

	/* Unbroken ward - can't move */
	return FALSE;
}

/**
 * Try to push past / kill another monster.  Returns TRUE on success.
 */
static bool process_monster_try_push(struct chunk *c, struct monster *m_ptr, const char *m_name, int nx, int ny)
{
	monster_type *n_ptr = square_monster(c, ny, nx);
	monster_lore *l_ptr = get_lore(m_ptr->race);

	/* Kill weaker monsters */
	int kill_ok = rf_has(m_ptr->race->flags, RF_KILL_BODY);

	/* Move weaker monsters if they can swap places */
	/* (not in a wall) */
	int move_ok = (rf_has(m_ptr->race->flags, RF_MOVE_BODY) &&
				   square_ispassable(c, m_ptr->fy, m_ptr->fx));

	if (compare_monsters(m_ptr, n_ptr) > 0) {
		/* Learn about pushing and shoving */
		if (m_ptr->ml) {
			rf_on(l_ptr->flags, RF_KILL_BODY);
			rf_on(l_ptr->flags, RF_MOVE_BODY);
		}

		if (kill_ok || move_ok) {
			/* Get the names of the monsters involved */
			char n_name[80];
			monster_desc(n_name, sizeof(n_name), n_ptr, MDESC_IND_HID);

			/* Reveal mimics */
			if (is_mimicking(n_ptr))
				become_aware(n_ptr);

			/* Monster ate another monster */
			if (kill_ok) {
				/* Note if visible */
				if (m_ptr->ml && (m_ptr->mflag & MFLAG_VIEW))
					msg("%s tramples over %s.", m_name, n_name);

				delete_monster(ny, nx);
			} else {
				/* Note if visible */
				if (m_ptr->ml && (m_ptr->mflag & MFLAG_VIEW))
					msg("%s pushes past %s.", m_name, n_name);
			}

			return TRUE;
		} 
	}

	return FALSE;
}

/**
 * Grab all objects from the grid.
 */
void process_monster_grab_objects(struct chunk *c, struct monster *m_ptr, 
		const char *m_name, int nx, int ny)
{
	monster_lore *l_ptr = get_lore(m_ptr->race);

	s16b this_o_idx, next_o_idx = 0;
	for (this_o_idx = c->o_idx[ny][nx]; this_o_idx; this_o_idx = next_o_idx) {
		object_type *o_ptr;

		/* Get the object */
		o_ptr = cave_object(c, this_o_idx);

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Skip gold */
		if (tval_is_money(o_ptr)) continue;

		/* Learn about item pickup behavior */
		if (m_ptr->ml) {
			rf_on(l_ptr->flags, RF_TAKE_ITEM);
			rf_on(l_ptr->flags, RF_KILL_ITEM);
		}

		/* Take or Kill objects on the floor */
		if (rf_has(m_ptr->race->flags, RF_TAKE_ITEM) ||
				rf_has(m_ptr->race->flags, RF_KILL_ITEM)) {
			char o_name[80];

			bool safe = o_ptr->artifact ? TRUE : FALSE;

			/* Get the object name */
			object_desc(o_name, sizeof(o_name), o_ptr, ODESC_PREFIX | ODESC_FULL);

			/* React to objects that hurt the monster */
			if (react_to_slay(o_ptr, m_ptr))
				safe = TRUE;

			/* The object cannot be picked up by the monster */
			if (safe) {
				/* Only give a message for "take_item" */
				if (rf_has(m_ptr->race->flags, RF_TAKE_ITEM) &&
							m_ptr->ml && player_has_los_bold(ny, nx) &&
							!ignore_item_ok(o_ptr)) {
					/* Dump a message */
					msg("%s tries to pick up %s, but fails.",
						m_name, o_name);
				}

			/* Pick up the item */
			} else if (rf_has(m_ptr->race->flags, RF_TAKE_ITEM)) {
				object_type *i_ptr;
				object_type object_type_body;

				/* Describe observable situations */
				if (player_has_los_bold(ny, nx) && !ignore_item_ok(o_ptr))
					msg("%s picks up %s.", m_name, o_name);

				/* Get local object */
				i_ptr = &object_type_body;

				/* Obtain local object */
				object_copy(i_ptr, o_ptr);

				/* Delete the object */
				delete_object_idx(this_o_idx);

				/* Carry the object */
				monster_carry(c, m_ptr, i_ptr);

			/* Destroy the item */
			} else {
				/* Describe observable situations */
				if (player_has_los_bold(ny, nx) && !ignore_item_ok(o_ptr))
					msgt(MSG_DESTROY, "%s crushes %s.", m_name, o_name);

				/* Delete the object */
				delete_object_idx(this_o_idx);
			}
		}
	}
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
static void process_monster(struct chunk *c, struct monster *m_ptr)
{
	monster_lore *l_ptr = get_lore(m_ptr->race);

	bool do_turn = FALSE;
	bool do_view = FALSE;

	int i;
	int mm[5];
	bool stagger = FALSE;
	char m_name[80];

	/* Get the monster name */
	monster_desc(m_name, sizeof(m_name), m_ptr, MDESC_CAPITAL | MDESC_IND_HID);

	/* Process timed effects - skip turn if necessary */
	if (process_monster_timed(c, m_ptr, m_name))
		return;

	/* Try to multiply - this can use up a turn */
	if (rf_has(m_ptr->race->flags, RF_MULTIPLY) && process_monster_multiply(c, m_ptr))
		return;

	/* Mimics lie in wait */
	if (is_mimicking(m_ptr)) return;

	/* Attempt to cast a spell */
	if (make_attack_spell(m_ptr)) return;

	/* Work out what kind of movement to use */
	if (process_monster_should_stagger(m_ptr)) {
		stagger = TRUE;
	} else {
		/* Logical moves, may do nothing */
		if (!get_moves(c, m_ptr, mm)) return;		
	}

	/* Process moves */
	for (i = 0; i < 5 && !do_turn; i++) {
		int ny, nx;

		int oy = m_ptr->fy;
		int ox = m_ptr->fx;

		/* Get the direction (or stagger) */
		int d = (stagger ? ddd[randint0(8)] : mm[i]);

		/* Get the destination */
		ny = oy + ddy[d];
		nx = ox + ddx[d];

		/* Check if we can move */
		if (!process_monster_can_move(c, m_ptr, m_name, nx, ny, &do_view, &do_turn))
			continue;

		/* Try to break the glyph if there is one */
		if (square_iswarded(c, ny, nx) && !process_monster_glyph(c, m_ptr, nx, ny))
			continue;

		/* The player is in the way. */
		if (square_isplayer(c, ny, nx)) {
			/* Learn about if the monster attacks */
			if (m_ptr->ml)
				rf_on(l_ptr->flags, RF_NEVER_BLOW);

			/* Some monsters never attack */
			if (rf_has(m_ptr->race->flags, RF_NEVER_BLOW))
				continue;

			/* Otherwise, attack the player */
			make_attack_normal(m_ptr, player);

			do_turn = TRUE;
			break;
		}

		/* Some monsters never move */
		/* XXX-AS Shouldn't this be a lot earlier? */
		if (rf_has(m_ptr->race->flags, RF_NEVER_MOVE)) {
			/* Learn about lack of movement */
			if (m_ptr->ml)
				rf_on(l_ptr->flags, RF_NEVER_MOVE);

			continue;
		}

		/* A monster is in the way, try pushing past/killing it */
		if (square_monster(c, ny, nx) && !process_monster_try_push(c, m_ptr, m_name, nx, ny))
			continue;

		/* If we got this far, then we can move, so do that. */
		monster_swap(oy, ox, ny, nx);

		/* Learn about no lack of movement */
		if (m_ptr->ml) rf_on(l_ptr->flags, RF_NEVER_MOVE);

		/* Possible disturb */
		if (m_ptr->ml && (m_ptr->mflag & MFLAG_VIEW) && OPT(disturb_near))
			disturb(player, 0);

		/* Scan all objects in the grid */
		process_monster_grab_objects(c, m_ptr, m_name, nx, ny);

		/* Take a turn */
		do_turn = TRUE;
		break;
	}

	if (rf_has(m_ptr->race->flags, RF_HAS_LIGHT))
		do_view = TRUE;

	/* Notice changes in view */
	if (do_view) {
		/* Update the visuals */
		player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);

		/* Fully update the flow XXX XXX XXX */
		player->upkeep->update |= (PU_FORGET_FLOW | PU_UPDATE_FLOW);
	}

	/* Hack -- get "bold" if out of options */
	if (!do_turn && m_ptr->m_timed[MON_TMD_FEAR])
		mon_clear_timed(m_ptr, MON_TMD_FEAR, MON_TMD_FLG_NOTIFY, FALSE);

	/* If we see an unaware monster do something, become aware of it */
	if (do_turn && m_ptr->unaware)
		become_aware(m_ptr);
}


static bool monster_can_flow(struct chunk *c, struct monster *m_ptr)
{
	int fy = m_ptr->fy;
	int fx = m_ptr->fx;

	assert(c);

	/* Check the flow (normal aaf is about 20) */
	if ((c->when[fy][fx] == c->when[player->py][player->px]) &&
	    (c->cost[fy][fx] < MONSTER_FLOW_DEPTH) &&
	    (c->cost[fy][fx] < (OPT(birth_small_range) ? m_ptr->race->aaf / 2 : m_ptr->race->aaf)))
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
void process_monsters(struct chunk *c, byte minimum_energy)
{
	int i;

	/* Process the monsters (backwards) */
	for (i = cave_monster_max(c) - 1; i >= 1; i--)
	{
		monster_type *m_ptr;

		/* Handle "leaving" */
		if (player->upkeep->leaving) break;

		/* Get a 'live' monster */
		m_ptr = cave_monster(cave, i);
		if (!m_ptr->race) continue;

		/* Not enough energy to move */
		if (m_ptr->energy < minimum_energy) continue;

		/* Use up "some" energy */
		m_ptr->energy -= 100;

		/* Set this monster to be the current actor */
		c->mon_current = i;

		/*
		 * Process the monster if the monster either:
		 * - can "sense" the player
		 * - is hurt
		 * - can "see" the player (checked backwards)
		 * - can "smell" the player from far away (flow)
		 */
		if ((m_ptr->cdis <= (OPT(birth_small_range) ? m_ptr->race->aaf / 2 : m_ptr->race->aaf)) ||
		    (m_ptr->hp < m_ptr->maxhp) ||
		    player_has_los_bold(m_ptr->fy, m_ptr->fx) ||
		    monster_can_flow(c, m_ptr)) {
			/* Process the monster */
			process_monster(c, m_ptr);
		}

		/* Monster is no longer current */
		c->mon_current = -1;
	}
}

/* Test functions */
bool (*testfn_make_attack_normal)(struct monster *m, struct player *p) = make_attack_normal;
