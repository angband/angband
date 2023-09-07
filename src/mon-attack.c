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
 * Given the monster, *mon, and cave *c, set *dist to the distance to the
 * monster's target and *grid to the target's location.  Accounts for a player
 * decoy, if present.  Either dist or grid may be NULL if that value is not
 * needed.
 */
static void monster_get_target_dist_grid(struct monster *mon, int *dist,
										 struct loc *grid)
{
	if (monster_is_decoyed(mon)) {
		struct loc decoy = cave_find_decoy(cave);
		if (dist) {
			*dist = distance(mon->grid, decoy);
		}
		if (grid) {
			*grid = decoy;
		}
	} else {
		if (dist) {
			*dist = mon->cdis;
		}
		if (grid) {
			*grid = player->grid;
		}
	}
}

/**
 * Check if a monster has a chance of casting a spell this turn
 */
static bool monster_can_cast(struct monster *mon, bool innate)
{
	int chance = innate ? mon->race->freq_innate : mon->race->freq_spell;
	int tdist;
	struct loc tgrid;

	monster_get_target_dist_grid(mon, &tdist, &tgrid);

	/* Cannot cast spells when nice */
	if (mflag_has(mon->mflag, MFLAG_NICE)) return false;

	/* Not allowed to cast spells */
	if (!chance) return false;

	/* Taunted monsters are likely just to attack */
	if (player->timed[TMD_TAUNT]) {
		chance /= 2;
	}

	/* Monsters at their preferred range are more likely to cast */
	if (tdist == mon->best_range) {
		chance *= 2;
	}

	/* Only do spells occasionally */
	if (randint0(100) >= chance) return false;

	/* Check range */
	if (tdist > z_info->max_range) return false;

	/* Check path */
	if (!projectable(cave, mon->grid, tgrid, PROJECT_SHORT))
		return false;

	/* If the target isn't the player, only cast if the player can witness */
	if ((tgrid.x != player->grid.x || tgrid.y != player->grid.y) &&
		!square_isview(cave, mon->grid) &&
		!square_isview(cave, tgrid)) {
		struct loc *path = mem_alloc(z_info->max_range * sizeof(*path));
		int npath, ipath;

		npath = project_path(cave, path, z_info->max_range, mon->grid,
			tgrid, PROJECT_SHORT);
		ipath = 0;
		while (1) {
			if (ipath >= npath) {
				/* No point on path visible.  Don't cast. */
				mem_free(path);
				return false;
			}
			if (square_isview(cave, path[ipath])) {
				break;
			}
			++ipath;
		}
		mem_free(path);
	}

	return true;
}

/**
 * Remove the "bad" spells from a spell list
 */
static void remove_bad_spells(struct monster *mon, bitflag f[RSF_SIZE])
{
	bitflag f2[RSF_SIZE];
	int tdist;

	monster_get_target_dist_grid(mon, &tdist, NULL);

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
	if (tdist == 1) {
		rsf_off(f2, RSF_TELE_TO);
		rsf_off(f2, RSF_TELE_SELF_TO);
	}

	/* Don't use the lash effect if the player is too far away */
	if (tdist > 2) {
		rsf_off(f2, RSF_WHIP);
	}
	if (tdist > 3) {
		rsf_off(f2, RSF_SPIT);
	}

	/* Update acquired knowledge */
	if (OPT(player, birth_ai_learn)) {
		size_t i;
		bitflag ai_flags[OF_SIZE], ai_pflags[PF_SIZE];
		struct element_info el[ELEM_MAX];
		bool know_something = false;

		/* Occasionally forget player status */
		if (one_in_(20)) {
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
static bool summon_possible(struct loc grid)
{
	int y, x;

	/* No summons in arenas */
	if (player->upkeep->arena_level) return false;

	/* Start at the location, and check 2 grids in each dir */
	for (y = grid.y - 2; y <= grid.y + 2; y++) {
		for (x = grid.x - 2; x <= grid.x + 2; x++) {
			struct loc near = loc(x, y);

			/* Ignore illegal locations */
			if (!square_in_bounds(cave, near)) continue;

			/* Only check a circular area */
			if (distance(grid, near) > 2) continue;

			/* Hack: no summon on glyph of warding */
			if (square_iswarded(cave, near)) continue;

			/* If it's empty floor grid in line of sight, we're good */
			if (square_isempty(cave, near) && los(cave, grid, near))
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
int choose_attack_spell(bitflag *f, bool innate, bool non_innate)
{
	int num = 0;
	uint8_t spells[RSF_MAX];

	int i;

	/* Paranoid initialization */
	for (i = 0; i < RSF_MAX; i++) {
		spells[i] = 0;
	}

	/* Extract spells, filtering as necessary */
	for (i = FLAG_START, num = 0; i < RSF_MAX; i++) {
		if (!innate && mon_spell_is_innate(i)) continue;
		if (!non_innate && !mon_spell_is_innate(i)) continue;
		if (rsf_has(f, i)) spells[num++] = i;
	}

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
 * Calculate the base to-hit value for a monster attack based on race only.
 * See also: chance_of_spell_hit_base
 *
 * \param race The monster race
 * \param effect The attack
 */
int chance_of_monster_hit_base(const struct monster_race *race,
	const struct blow_effect *effect)
{
	return MAX(race->level, 1) * 3 + effect->power;
}

/**
 * Calculate the to-hit value of a monster attack for a specific monster
 *
 * \param mon The monster
 * \param effect The attack
 */
static int chance_of_monster_hit(const struct monster *mon,
	const struct blow_effect *effect)
{
	int to_hit = chance_of_monster_hit_base(mon->race, effect);

	/* Apply stun hit reduction if applicable */
	if (mon->m_timed[MON_TMD_STUN]) {
		to_hit = to_hit * (100 - STUN_HIT_REDUCTION) / 100;
	}

	return to_hit;
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
 *
 * Note the interaction between innate attacks and non-innate attacks (true
 * spells).  Because the check for spells is done first, actual innate attack
 * frequencies are affected by the spell frequency.
 */
bool make_ranged_attack(struct monster *mon)
{
	struct monster_lore *lore = get_lore(mon->race);
	int thrown_spell, failrate;
	bitflag f[RSF_SIZE];
	char m_name[80];
	bool seen = (player->timed[TMD_BLIND] == 0) && monster_is_visible(mon);
	bool innate = false;

	/* Check for cast this turn, non-innate and then innate */
	if (!monster_can_cast(mon, false)) {
		if (!monster_can_cast(mon, true)) {
			return false;
		} else {
			/* We're casting an innate "spell" */
			innate = true;
		}
	}

	/* Extract the racial spell flags */
	rsf_copy(f, mon->race->spell_flags);

	/* Smart monsters can use "desperate" spells */
	if (monster_is_smart(mon) && mon->hp < mon->maxhp / 10 && one_in_(2)) {
		ignore_spells(f, RST_DAMAGE);
	}

	/* Non-stupid monsters do some filtering */
	if (!monster_is_stupid(mon)) {
		struct loc tgrid;

		/* Remove the "ineffective" spells */
		remove_bad_spells(mon, f);

		/* Check for a clean bolt shot */
		monster_get_target_dist_grid(mon, NULL, &tgrid);
		if (test_spells(f, RST_BOLT) &&
			!projectable(cave, mon->grid, tgrid, PROJECT_STOP)) {
			ignore_spells(f, RST_BOLT);
		}

		/* Check for a possible summon */
		if (!summon_possible(mon->grid)) {
			ignore_spells(f, RST_SUMMON);
		}
	}

	/* No spells left */
	if (rsf_is_empty(f)) return false;

	/* Choose a spell to cast */
	thrown_spell = choose_attack_spell(f, innate, !innate);

	/* Abort if no spell was chosen */
	if (!thrown_spell) return false;

	/* There will be at least an attempt now, so get the monster's name */
	monster_desc(m_name, sizeof(m_name), mon, MDESC_STANDARD);

	/* If we see a hidden monster try to cast a spell, become aware of it */
	if (monster_is_camouflaged(mon))
		become_aware(cave, mon);

	/* Check for spell failure (innate attacks never fail) */
	failrate = monster_spell_failrate(mon);
	if (!mon_spell_is_innate(thrown_spell) && (randint0(100) < failrate)) {
		msg("%s tries to cast a spell, but fails.", m_name);
		return true;
	}

	/* Cast the spell. */
	disturb(player);
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
 * Determine if an attack against the player succeeds.
 */
bool check_hit(struct player *p, int to_hit)
{
	/* If anything checks vs ac, the player learns ac bonuses */
	equip_learn_on_defend(p);

	/* Check if the player was hit */
	return test_hit(to_hit, p->state.ac + p->state.to_a);
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

	/* Not allowed to attack */
	if (rf_has(mon->race->flags, RF_NEVER_BLOW)) return (false);

	/* Get the monster name (or "it") */
	monster_desc(m_name, sizeof(m_name), mon, MDESC_STANDARD);

	/* Get the "died from" information (i.e. "a kobold") */
	monster_desc(ddesc, sizeof(ddesc), mon, MDESC_SHOW | MDESC_IND_VIS);

	/* Scan through all blows */
	for (ap_cnt = 0; ap_cnt < z_info->mon_blows_max; ap_cnt++) {
		struct loc pgrid = p->grid;
		bool visible = monster_is_visible(mon) || (mon->race->light > 0);
		bool obvious = false;

		int damage = 0;
		bool do_cut = false;
		bool do_stun = false;
		int sound_msg = MSG_GENERIC;

		char *act = NULL;

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
			check_hit(p, chance_of_monster_hit(mon, effect))) {
			melee_effect_handler_f effect_handler;

			/* Always disturbing */
			disturb(p);

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
			act = monster_blow_method_action(method, -1);
			do_cut = method->cut;
			do_stun = method->stun;
			sound_msg = method->msgt;

			/* Hack -- assume all attacks are obvious */
			obvious = true;

			/* Roll dice */
			damage = randcalc(dice, rlev, RANDOMISE);

			/* Reduce damage when stunned */
			if (mon->m_timed[MON_TMD_STUN]) {
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
					NULL,
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
				if (amt) {
					(void)player_inc_timed(p, TMD_CUT, amt,
						true, true, true);
				}
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
				if (amt) {
					(void)player_inc_timed(p, TMD_STUN, amt,
						true, true, true);
				}
			}

			string_free(act);
		} else {
			/* Visible monster missed player, so notify if appropriate. */
			if (monster_is_visible(mon) &&	method->miss) {
				/* Disturbing */
				disturb(p);
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
		if (!loc_eq(p->grid, pgrid)) break;
	}

	/* Blink away */
	if (blinked) {
		char dice[5];

		if (!p->is_dead && square_isseen(cave, mon->grid)) {
			add_monster_message(mon, MON_MSG_HIT_AND_RUN, true);
		}
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

/**
 * Attack another monster via physical attacks.
 */
bool monster_attack_monster(struct monster *mon, struct monster *t_mon)
{
	struct monster_lore *lore = get_lore(mon->race);
	int rlev = ((mon->race->level >= 1) ? mon->race->level : 1);
	int ap_cnt;
	char m_name[80];
	char t_name[80];
	bool blinked = false;

	/* Not allowed to attack */
	if (rf_has(mon->race->flags, RF_NEVER_BLOW)) return (false);

	/* Get the monster names (or "it") */
	monster_desc(m_name, sizeof(m_name), mon, MDESC_STANDARD);
	monster_desc(t_name, sizeof(t_name), t_mon, MDESC_TARG);

	/* Scan through all blows */
	for (ap_cnt = 0; ap_cnt < z_info->mon_blows_max; ap_cnt++) {
		struct loc grid = t_mon->grid;
		bool visible = monster_is_visible(mon) || (mon->race->light > 0);
		bool obvious = false;

		int damage = 0;
		bool do_stun = false;
		int sound_msg = MSG_GENERIC;

		char *act = NULL;

		/* Extract the attack infomation */
		struct blow_effect *effect = mon->race->blow[ap_cnt].effect;
		struct blow_method *method = mon->race->blow[ap_cnt].method;
		random_value dice = mon->race->blow[ap_cnt].dice;

		/* No more attacks */
		if (!method) break;

		/* Monster hits monster */
		assert(effect);
		if (streq(effect->name, "NONE") ||
			test_hit(chance_of_monster_hit(mon, effect), t_mon->race->ac)) {
			melee_effect_handler_f effect_handler;

			/* Describe the attack method */
			act = monster_blow_method_action(method, t_mon->midx);
			do_stun = method->stun;
			sound_msg = method->msgt;

			/* Hack -- assume all attacks are obvious */
			obvious = true;

			/* Roll dice */
			damage = randcalc(dice, rlev, RANDOMISE);

			/* Reduce damage when stunned */
			if (mon->m_timed[MON_TMD_STUN]) {
				damage = (damage * (100 - STUN_DAM_REDUCTION)) / 100;
			}

			/* Message */
			if (act) {
				const char *fullstop = ".";
				if (suffix(act, "'") || suffix(act, "!")) {
					fullstop = "";
				}

				msgt(sound_msg, "%s %s%s", m_name, act, fullstop);
			}

			/* Perform the actual effect. */
			effect_handler = melee_handler_for_blow_effect(effect->name);
			if (effect_handler != NULL) {
				melee_effect_handler_context_t context = {
					NULL,
					mon,
					t_mon,
					rlev,
					method,
					t_mon->race->ac,
					NULL,
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

			/* Handle stun */
			if (do_stun && square_monster(cave, grid)) {
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
					(void)mon_inc_timed(t_mon, MON_TMD_STUN, amt, 0);
			}

			string_free(act);
		} else {
			/* Visible monster missed monster, so notify if appropriate. */
			if (monster_is_visible(mon) && method->miss) {
				msg("%s misses %s.", m_name, t_name);
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

		/* Skip the other blows if the target has moved or died */
		if (!square_monster(cave, grid)) break;
	}

	/* Blink away */
	if (blinked) {
		char dice[5];

		if (square_isseen(cave, mon->grid)) {
			add_monster_message(mon, MON_MSG_HIT_AND_RUN, true);
		}
		strnfmt(dice, sizeof(dice), "%d", z_info->max_sight * 2 + 5);
		effect_simple(EF_TELEPORT, source_monster(mon->midx), dice, 0, 0, 0, 0, 0, NULL);
	}

	/* Learn lore */
	lore_update(mon->race, lore);

	/* Assume we attacked */
	return (true);
}
