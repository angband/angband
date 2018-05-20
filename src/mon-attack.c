/**
 * \file mon-attack.c
 * \brief Monster attacks
 *
 * Monster ranged attacks - choosing an attack spell or shot and making it.
 * Monster melee attacks - monster critical blows, whether a monster 
 * attack hits, what happens when a monster attacks an adjacent player.
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
#include "cave.h"
#include "effects.h"
#include "init.h"
#include "mon-attack.h"
#include "mon-blows.h"
#include "mon-desc.h"
#include "mon-lore.h"
#include "mon-predicate.h"
#include "mon-spell.h"
#include "mon-timed.h"
#include "mon-util.h"
#include "obj-knowledge.h"
#include "player-attack.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"

/**
 * This file deals with monster attacks (including spells) as follows:
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
 */

/**
 * Check if a monster has a chance of casting a spell this turn
 */
bool monster_can_cast(struct monster *mon)
{
	int chance = (mon->race->freq_innate + mon->race->freq_spell) / 2;
	int px = player->px;
	int py = player->py;

	/* Cannot cast spells when nice */
	if (mflag_has(mon->mflag, MFLAG_NICE)) return false;

	/* Not allowed to cast spells */
	if (!chance) return false;

	/* Only do spells occasionally */
	if (randint0(100) >= chance) return false;

	/* Check range */
	if (mon->cdis > z_info->max_range) return false;

	/* Check path */
	if (!projectable(cave, mon->fy, mon->fx, py, px, PROJECT_NONE))
		return false;

	return true;
}

/**
 * Remove the "bad" spells from a spell list
 */
static void remove_bad_spells(struct monster *mon, bitflag f[RSF_SIZE])
{
	bitflag f2[RSF_SIZE];

	/* Take working copy of spell flags */
	rsf_copy(f2, f);

	/* Don't heal if full */
	if (mon->hp >= mon->maxhp) {
		rsf_off(f2, RSF_HEAL);
	}

	/* Don't heal others if no injuries */
	if (rsf_has(f2, RSF_HEAL_KIN) && !find_any_nearby_injured_kin(cave, mon)) {
		rsf_off(f2, RSF_HEAL_KIN);
	}

	/* Don't haste if hasted with time remaining */
	if (mon->m_timed[MON_TMD_FAST] > 10) {
		rsf_off(f2, RSF_HASTE);
	}

	/* Don't teleport to if the player is already next to us */
	if (mon->cdis == 1) {
		rsf_off(f2, RSF_TELE_TO);
	}

	/* Update acquired knowledge */
	if (OPT(player, birth_ai_learn)) {
		size_t i;
		bitflag ai_flags[OF_SIZE], ai_pflags[PF_SIZE];
		struct element_info el[ELEM_MAX];
		bool know_something = false;

		/* Occasionally forget player status */
		if (one_in_(100)) {
			of_wipe(mon->known_pstate.flags);
			pf_wipe(mon->known_pstate.pflags);
			for (i = 0; i < ELEM_MAX; i++)
				mon->known_pstate.el_info[i].res_level = 0;
		}

		/* Use the memorized info */
		of_wipe(ai_flags);
		pf_wipe(ai_pflags);
		of_copy(ai_flags, mon->known_pstate.flags);
		pf_copy(ai_pflags, mon->known_pstate.pflags);
		if (!of_is_empty(ai_flags) || !pf_is_empty(ai_pflags)) {
			know_something = true;
		}

		for (i = 0; i < ELEM_MAX; i++) {
			el[i].res_level = mon->known_pstate.el_info[i].res_level;
			if (el[i].res_level != 0) {
				know_something = true;
			}
		}

		/* Cancel out certain flags based on knowledge */
		if (know_something) {
			unset_spells(f2, ai_flags, ai_pflags, el, mon);
		}
	}

	/* Use working copy of spell flags */
	rsf_copy(f, f2);
}


/**
 * Determine if there is a space near the selected spot in which
 * a summoned creature can appear
 */
static bool summon_possible(int y1, int x1)
{
	int y, x;

	/* Start at the location, and check 2 grids in each dir */
	for (y = y1 - 2; y <= y1 + 2; y++) {
		for (x = x1 - 2; x <= x1 + 2; x++) {
			/* Ignore illegal locations */
			if (!square_in_bounds(cave, y, x)) continue;

			/* Only check a circular area */
			if (distance(loc(x1, y1), loc(x, y)) > 2) continue;

			/* Hack: no summon on glyph of warding */
			if (square_iswarded(cave, y, x)) continue;

			/* If it's empty floor grid in line of sight, we're good */
			if (square_isempty(cave, y, x) && los(cave, y1, x1, y, x))
				return (true);
		}
	}

	return false;
}


/**
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
int choose_attack_spell(bitflag *f)
{
	int num = 0;
	byte spells[RSF_MAX];

	int i;

	/* Extract all spells: "innate", "normal", "bizarre" */
	for (i = FLAG_START, num = 0; i < RSF_MAX; i++)
		if (rsf_has(f, i)) spells[num++] = i;

	/* Paranoia */
	if (num == 0) return 0;

	/* Pick at random */
	return (spells[randint0(num)]);
}

/**
 * Failure rate of a monster's spell, based on spell power and current status
 */
static int monster_spell_failrate(struct monster *mon)
{
	int power = MIN(mon->race->spell_power, 1);
	int failrate = 0;

	/* Stupid monsters will never fail (for jellies and such) */
	if (!monster_is_stupid(mon)) {
		/* Base failrate */
		failrate = 25 - (power + 3) / 4;

		/* Fear adds 20% */
		if (mon->m_timed[MON_TMD_FEAR])
			failrate += 20;

		/* Confusion and diesnchantment add 50% */
		if (mon->m_timed[MON_TMD_CONF] || mon->m_timed[MON_TMD_DISEN])
			failrate += 50;
	}

	return failrate;
}

/**
 * Creatures can cast spells, shoot missiles, and breathe.
 *
 * Returns "true" if a spell (or whatever) was (successfully) cast.
 *
 * Perhaps monsters should breathe at locations *near* the player,
 * since this would allow them to inflict "partial" damage.
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
bool make_attack_spell(struct monster *mon)
{
	struct monster_lore *lore = get_lore(mon->race);
	int thrown_spell, failrate;
	bitflag f[RSF_SIZE];
	char m_name[80];
	int px = player->px;
	int py = player->py;
	bool seen = (player->timed[TMD_BLIND] == 0) && monster_is_visible(mon);

	/* Check prerequisites */
	if (!monster_can_cast(mon)) return false;

	/* Extract the racial spell flags */
	rsf_copy(f, mon->race->spell_flags);

	/* Smart monsters can use "desperate" spells */
	if (monster_is_smart(mon) && mon->hp < mon->maxhp / 10 && one_in_(2)) {
		ignore_spells(f, RST_DAMAGE);
	}

	/* Non-stupid monsters do some filtering */
	if (!monster_is_stupid(mon)) {
		/* Remove the "ineffective" spells */
		remove_bad_spells(mon, f);

		/* Check for a clean bolt shot */
		if (test_spells(f, RST_BOLT) &&
			!projectable(cave, mon->fy, mon->fx, py, px, PROJECT_STOP)) {
			ignore_spells(f, RST_BOLT);
		}

		/* Check for a possible summon */
		if (!summon_possible(mon->fy, mon->fx)) {
			ignore_spells(f, RST_SUMMON);
		}
	}

	/* No spells left */
	if (rsf_is_empty(f)) return false;

	/* Choose a spell to cast */
	thrown_spell = choose_attack_spell(f);

	/* Abort if no spell was chosen */
	if (!thrown_spell) return false;

	/* There will be at least an attempt now, so get the monster's name */
	monster_desc(m_name, sizeof(m_name), mon, MDESC_STANDARD);

	/* If we see a hidden monster try to cast a spell, become aware of it */
	if (monster_is_camouflaged(mon))
		become_aware(mon);

	/* Check for spell failure (innate attacks never fail) */
	failrate = monster_spell_failrate(mon);
	if (!mon_spell_is_innate(thrown_spell) && (randint0(100) < failrate)) {
		msg("%s tries to cast a spell, but fails.", m_name);
		return true;
	}

	/* Cast the spell. */
	disturb(player, 1);
	do_mon_spell(thrown_spell, mon, seen);

	/* Remember what the monster did */
	if (seen) {
		rsf_on(lore->spell_flags, thrown_spell);
		if (mon_spell_is_innate(thrown_spell)) {
			/* Innate spell */
			if (lore->cast_innate < UCHAR_MAX)
				lore->cast_innate++;
		} else {
			/* Bolt or Ball, or Special spell */
			if (lore->cast_spell < UCHAR_MAX)
				lore->cast_spell++;
		}
	}
	if (player->is_dead && (lore->deaths < SHRT_MAX)) {
		lore->deaths++;
	}
	lore_update(mon->race, lore);

	/* A spell was cast */
	return true;
}



/**
 * Critical blow.  All hits that do 95% of total possible damage,
 * and which also do at least 20 damage, or, sometimes, N damage.
 * This is used only to determine "cuts" and "stuns".
 */
static int monster_critical(random_value dice, int rlev, int dam)
{
	int max = 0;
	int total = randcalc(dice, rlev, MAXIMISE);

	/* Must do at least 95% of perfect */
	if (dam < total * 19 / 20) return (0);

	/* Weak blows rarely work */
	if ((dam < 20) && (randint0(100) >= dam)) return (0);

	/* Perfect damage */
	if (dam == total) max++;

	/* Super-charge */
	if (dam >= 20)
		while (randint0(100) < 2) max++;

	/* Critical damage */
	if (dam > 45) return (6 + max);
	if (dam > 33) return (5 + max);
	if (dam > 25) return (4 + max);
	if (dam > 18) return (3 + max);
	if (dam > 11) return (2 + max);
	return (1 + max);
}

/**
 * Determine if a monster attack against the player succeeds.
 */
bool check_hit(struct player *p, int power, int level, int accuracy)
{
	int chance, ac;

	/* Calculate the "attack quality" */
	chance = (power + (level * 3));

	/* Total armor */
	ac = p->state.ac + p->state.to_a;

	/* If the monster checks vs ac, the player learns ac bonuses */
	equip_learn_on_defend(p);

	/* Apply accuracy */
	chance *= accuracy;
	chance /= 100;

	/* Check if the player was hit */
	return test_hit(chance, ac, true);
}

/**
 * Determine if a monster attack against a monster succeeds.
 */
bool check_hit_monster(struct monster *mon, int power, int level, int accuracy)
{
	int chance, ac;

	/* Calculate the "attack quality" */
	chance = (power + (level * 3));

	/* Total armor */
	ac = mon->race->ac;

	/* Apply accuracy */
	chance *= accuracy;
	chance /= 100;

	/* Check if the monster was hit */
	return test_hit(chance, ac, true);
}

/**
 * Calculate how much damage remains after armor is taken into account
 * (does for a physical attack what adjust_dam does for an elemental attack).
 */
int adjust_dam_armor(int damage, int ac)
{
	return damage - (damage * ((ac < 240) ? ac : 240) / 400);
}

/**
 * Attack the player via physical attacks.
 */
bool make_attack_normal(struct monster *mon, struct player *p)
{
	struct monster_lore *lore = get_lore(mon->race);
	int rlev = ((mon->race->level >= 1) ? mon->race->level : 1);
	int ap_cnt;
	char m_name[80];
	char ddesc[80];
	bool blinked = false;
	bool stunned = mon->m_timed[MON_TMD_STUN] ? true : false;

	/* Not allowed to attack */
	if (rf_has(mon->race->flags, RF_NEVER_BLOW)) return (false);

	/* Get the monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), mon, MDESC_STANDARD);

	/* Get the "died from" information (i.e. "a kobold") */
	monster_desc(ddesc, sizeof(ddesc), mon, MDESC_SHOW | MDESC_IND_VIS);

	/* Scan through all blows */
	for (ap_cnt = 0; ap_cnt < z_info->mon_blows_max; ap_cnt++) {
		int py = p->py, px = p->px;
		bool visible = monster_is_visible(mon) ||
			rf_has(mon->race->flags, RF_HAS_LIGHT);
		bool obvious = false;

		int damage = 0;
		bool do_cut = false;
		bool do_stun = false;
		int sound_msg = MSG_GENERIC;

		const char *act = NULL;

		/* Extract the attack infomation */
		struct blow_effect *effect = mon->race->blow[ap_cnt].effect;
		struct blow_method *method = mon->race->blow[ap_cnt].method;
		random_value dice = mon->race->blow[ap_cnt].dice;

		/* No more attacks */
		if (!method) break;

		/* Handle "leaving" */
		if (p->is_dead || p->upkeep->generate_level) break;

		/* Monster hits player */
		assert(effect);
		if (streq(effect->name, "NONE") ||
				check_hit(p, effect->power, rlev,
						  stunned ? STUN_HIT_REDUCTION : 0)) {
			melee_effect_handler_f effect_handler;

			/* Always disturbing */
			disturb(p, 1);

			/* Hack -- Apply "protection from evil" */
			if (p->timed[TMD_PROTEVIL] > 0) {
				/* Learn about the evil flag */
				if (monster_is_visible(mon))
					rf_on(lore->flags, RF_EVIL);

				if (monster_is_evil(mon) && p->lev >= rlev &&
				    randint0(100) + p->lev > 50) {
					/* Message */
					msg("%s is repelled.", m_name);

					/* Hack -- Next attack */
					continue;
				}
			}

			/* Describe the attack method */
			act = monster_blow_method_action(method);
			do_cut = method->cut;
			do_stun = method->stun;
			sound_msg = method->msgt;

			/* Hack -- assume all attacks are obvious */
			obvious = true;

			/* Roll dice */
			damage = randcalc(dice, rlev, RANDOMISE);

			/* Reduce damage when stunned */
			if (stunned) {
				damage = (damage * (100 - STUN_DAM_REDUCTION)) / 100;
			}

			/* Message */
			if (act) {
				const char *fullstop = ".";
				if (suffix(act, "'") || suffix(act, "!")) {
					fullstop = "";
				}

				if (OPT(p, show_damage)) {
					msgt(sound_msg, "%s %s (%d)%s", m_name, act, damage,
						 fullstop);
				} else {
					msgt(sound_msg, "%s %s%s", m_name, act, fullstop);
				}
			}

			/* Perform the actual effect. */
			effect_handler = melee_handler_for_blow_effect(effect->name);
			if (effect_handler != NULL) {
				melee_effect_handler_context_t context = {
					p,
					mon,
					rlev,
					method,
					p->state.ac + p->state.to_a,
					ddesc,
					obvious,
					blinked,
					damage,
				};

				effect_handler(&context);

				/* Save any changes made in the handler for later use. */
				obvious = context.obvious;
				blinked = context.blinked;
				damage = context.damage;
			} else {
				msg("ERROR: Effect handler not found for %s.", effect->name);
			}

			/* Don't cut or stun if player is dead */
			if (p->is_dead) {
				do_cut = false;
				do_stun = false;
			}

			/* Hack -- only one of cut or stun */
			if (do_cut && do_stun) {
				/* Cancel cut */
				if (randint0(100) < 50)
					do_cut = false;

				/* Cancel stun */
				else
					do_stun = false;
			}

			/* Handle cut */
			if (do_cut) {
				/* Critical hit (zero if non-critical) */
				int amt, tmp = monster_critical(dice, rlev, damage);

				/* Roll for damage */
				switch (tmp) {
					case 0: amt = 0; break;
					case 1: amt = randint1(5); break;
					case 2: amt = randint1(5) + 5; break;
					case 3: amt = randint1(20) + 20; break;
					case 4: amt = randint1(50) + 50; break;
					case 5: amt = randint1(100) + 100; break;
					case 6: amt = 300; break;
					default: amt = 500; break;
				}

				/* Apply the cut */
				if (amt) (void)player_inc_timed(p, TMD_CUT, amt, true, true);
			}

			/* Handle stun */
			if (do_stun) {
				/* Critical hit (zero if non-critical) */
				int amt, tmp = monster_critical(dice, rlev, damage);

				/* Roll for damage */
				switch (tmp) {
					case 0: amt = 0; break;
					case 1: amt = randint1(5); break;
					case 2: amt = randint1(10) + 10; break;
					case 3: amt = randint1(20) + 20; break;
					case 4: amt = randint1(30) + 30; break;
					case 5: amt = randint1(40) + 40; break;
					case 6: amt = 100; break;
					default: amt = 200; break;
				}

				/* Apply the stun */
				if (amt)
					(void)player_inc_timed(p, TMD_STUN, amt, true, true);
			}
		} else {
			/* Visible monster missed player, so notify if appropriate. */
			if (monster_is_visible(mon) &&	method->miss) {
				/* Disturbing */
				disturb(p, 1);
				msg("%s misses you.", m_name);
			}
		}

		/* Analyze "visible" monsters only */
		if (visible) {
			/* Count "obvious" attacks (and ones that cause damage) */
			if (obvious || damage || (lore->blows[ap_cnt].times_seen > 10)) {
				/* Count attacks of this type */
				if (lore->blows[ap_cnt].times_seen < UCHAR_MAX)
					lore->blows[ap_cnt].times_seen++;
			}
		}

		/* Skip the other blows if the player has moved */
		if ((p->py != py) || (p->px != px)) break;
	}

	/* Blink away */
	if (blinked) {
		char dice[5];
		msg("There is a puff of smoke!");
		strnfmt(dice, sizeof(dice), "%d", z_info->max_sight * 2 + 5);
		effect_simple(EF_TELEPORT, source_monster(mon->midx), dice, 0, 0, 0, 0, 0, NULL);
	}

	/* Always notice cause of death */
	if (p->is_dead && (lore->deaths < SHRT_MAX))
		lore->deaths++;

	/* Learn lore */
	lore_update(mon->race, lore);

	/* Assume we attacked */
	return (true);
}
