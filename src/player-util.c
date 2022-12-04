/**
 * \file player-util.c
 * \brief Player utility functions
 *
 * Copyright (c) 2011 The Angband Developers. See COPYING.
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
#include "cmd-core.h"
#include "game-input.h"
#include "game-world.h"
#include "generate.h"
#include "init.h"
#include "obj-chest.h"
#include "obj-gear.h"
#include "obj-ignore.h"
#include "obj-knowledge.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-attack.h"
#include "player-calcs.h"
#include "player-history.h"
#include "player-quest.h"
#include "player-spell.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"
#include "score.h"
#include "store.h"
#include "target.h"
#include "trap.h"
#include "ui-input.h"

/**
 * Increment to the next or decrement to the preceeding level
   accounting for the stair skip value in constants
   Keep in mind to check all intermediate level for unskippable
   quests
*/
int dungeon_get_next_level(struct player *p, int dlev, int added)
{
	int target_level, i;

	/* Get target level */
	target_level = dlev + added * z_info->stair_skip;

	/* Don't allow levels below max */
	if (target_level > z_info->max_depth - 1)
		target_level = z_info->max_depth - 1;

	/* Don't allow levels above the town */
	if (target_level < 0) target_level = 0;

	/* Check intermediate levels for quests */
	for (i = dlev; i <= target_level; i++) {
		if (is_quest(p, i)) return i;
	}

	return target_level;
}

/**
 * Set recall depth for a player recalling from town
 */
void player_set_recall_depth(struct player *p)
{
	/* Account for forced descent */
	if (OPT(p, birth_force_descend)) {
		/* Force descent to a lower level if allowed */
		if (p->max_depth < z_info->max_depth - 1
				&& !is_quest(p, p->max_depth)) {
			p->recall_depth = dungeon_get_next_level(p,
				p->max_depth, 1);
		}
	}

	/* Players who haven't left town before go to level 1 */
	p->recall_depth = MAX(p->recall_depth, 1);
}

/**
 * Give the player the choice of persistent level to recall to.  Note that if
 * a level greater than the player's maximum depth is chosen, we silently go
 * to the maximum depth.
 */
bool player_get_recall_depth(struct player *p)
{
	bool level_ok = false;
	int new = 0;

	while (!level_ok) {
		const char *prompt =
			"Which level do you wish to return to (0 to cancel)? ";
		int i;

		/* Choose the level */
		new = get_quantity(prompt, p->max_depth);
		if (new == 0) {
			return false;
		}

		/* Is that level valid? */
		for (i = 0; i < chunk_list_max; i++) {
			if (chunk_list[i]->depth == new) {
				level_ok = true;
				break;
			}
		}
		if (!level_ok) {
			msg("You must choose a level you have previously visited.");
		}
	}
	p->recall_depth = new;
	return true;
}

/**
 * Change dungeon level - e.g. by going up stairs or with WoR.
 */
void dungeon_change_level(struct player *p, int dlev)
{
	/* New depth */
	p->depth = dlev;

	/* If we're returning to town, update the store contents
	   according to how long we've been away */
	if (!dlev && daycount)
		store_update();

	/* Leaving, make new level */
	p->upkeep->generate_level = true;

	/* Save the game when we arrive on the new level. */
	p->upkeep->autosave = true;
}


/**
 * Decreases players hit points and sets death flag if necessary
 *
 * Hack -- this function allows the user to save (or quit) the game
 * when he dies, since the "You die." message is shown before setting
 * the player to "dead".
 */
void take_hit(struct player *p, int dam, const char *kb_str)
{
	int old_chp = p->chp;

	int warning = (p->mhp * p->opts.hitpoint_warn / 10);

	/* Paranoia */
	if (p->is_dead) return;

	/* Mega-Hack -- Apply "invulnerability" */
	if (p->timed[TMD_INVULN] && (dam < 9000)) return;

	/* Apply damage reduction */
	dam -= p->state.dam_red;
	if (p->state.perc_dam_red) {
		dam -= (dam * p->state.perc_dam_red) / 100 ;
	}
	if (dam <= 0) return;

	/* Disturb */
	disturb(p);

	/* Hurt the player */
	p->chp -= dam;

	/* Reward COMBAT_REGEN characters with mana for their lost hitpoints
	 * Unenviable task of separating what should and should not cause rage
	 * If we eliminate the most exploitable cases it should be fine.
	 * All traps and lava currently give mana, which could be exploited  */
	if (player_has(p, PF_COMBAT_REGEN)  && !streq(kb_str, "poison")
		&& !streq(kb_str, "a fatal wound") && !streq(kb_str, "starvation")) {
		/* lose X% of hitpoints get X% of spell points */
		int32_t sp_gain = (((int32_t)MAX(p->msp, 10)) * 65536)
			/ (int32_t)p->mhp * dam;
		player_adjust_mana_precise(p, sp_gain);
	}

	/* Display the hitpoints */
	p->upkeep->redraw |= (PR_HP);

	/* Dead player */
	if (p->chp < 0) {
		/* From hell's heart I stab at thee */
		if (p->timed[TMD_BLOODLUST]
			&& (p->chp + p->timed[TMD_BLOODLUST] + p->lev >= 0)) {
			if (randint0(10)) {
				msg("Your lust for blood keeps you alive!");
			} else {
				msg("So great was his prowess and skill in warfare, the Elves said: ");
				msg("'The Mormegil cannot be slain, save by mischance.'");
			}
		} else if ((p->wizard || OPT(p, cheat_live)) && !get_check("Die? ")) {
			event_signal(EVENT_CHEAT_DEATH);
		} else {
			/* Hack -- Note death */
			msgt(MSG_DEATH, "You die.");
			event_signal(EVENT_MESSAGE_FLUSH);

			/* Note cause of death */
			my_strcpy(p->died_from, kb_str, sizeof(p->died_from));

			/* No longer a winner */
			p->total_winner = false;

			/* Note death */
			p->is_dead = true;

			/* Dead */
			return;
		}
	}

	/* Hitpoint warning */
	if (p->chp < warning) {
		/* Hack -- bell on first notice */
		if (old_chp > warning)
			bell();

		/* Message */
		msgt(MSG_HITPOINT_WARN, "*** LOW HITPOINT WARNING! ***");
		event_signal(EVENT_MESSAGE_FLUSH);
	}
}

/**
 * Win or not, know inventory, home items and history upon death, enter score
 */
void death_knowledge(struct player *p)
{
	struct store *home = &stores[f_info[FEAT_HOME].shopnum - 1];
	struct object *obj;
	time_t death_time = (time_t)0;

	/* Retire in the town in a good state */
	if (p->total_winner) {
		p->depth = 0;
		my_strcpy(p->died_from, "Ripe Old Age", sizeof(p->died_from));
		p->exp = p->max_exp;
		p->lev = p->max_lev;
		p->au += 10000000L;
	}

	player_learn_all_runes(p);
	for (obj = p->gear; obj; obj = obj->next) {
		object_flavor_aware(p, obj);
		obj->known->effect = obj->effect;
		obj->known->activation = obj->activation;
	}

	for (obj = home->stock; obj; obj = obj->next) {
		object_flavor_aware(p, obj);
		obj->known->effect = obj->effect;
		obj->known->activation = obj->activation;
	}

	history_unmask_unknown(p);

	/* Get time of death */
	(void)time(&death_time);
	enter_score(p, &death_time);

	/* Hack -- Recalculate bonuses */
	p->upkeep->update |= (PU_BONUS);
	handle_stuff(p);
}

/**
 * Energy per move, taking extra moves into account
 */
int energy_per_move(struct player *p)
{
	int num = p->state.num_moves;
	int energy = z_info->move_energy;
	return (energy * (1 + ABS(num) - num)) / (1 + ABS(num));
}

/**
 * Modify a stat value by a "modifier", return new value
 *
 * Stats go up: 3,4,...,17,18,18/10,18/20,...,18/220
 * Or even: 18/13, 18/23, 18/33, ..., 18/220
 *
 * Stats go down: 18/220, 18/210,..., 18/10, 18, 17, ..., 3
 * Or even: 18/13, 18/03, 18, 17, ..., 3
 */
int16_t modify_stat_value(int value, int amount)
{
	int i;

	/* Reward or penalty */
	if (amount > 0) {
		/* Apply each point */
		for (i = 0; i < amount; i++) {
			/* One point at a time */
			if (value < 18) value++;

			/* Ten "points" at a time */
			else value += 10;
		}
	} else if (amount < 0) {
		/* Apply each point */
		for (i = 0; i < (0 - amount); i++) {
			/* Ten points at a time */
			if (value >= 18+10) value -= 10;

			/* Hack -- prevent weirdness */
			else if (value > 18) value = 18;

			/* One point at a time */
			else if (value > 3) value--;
		}
	}

	/* Return new value */
	return (value);
}

/**
 * Swap player's stats at random, retaining information so they can be
 * reverted to their original state.
 */
void player_scramble_stats(struct player *p)
{
	int max1, cur1, max2, cur2, i, j, swap;

	/* Fisher-Yates shuffling algorithm */
	for (i = STAT_MAX - 1; i > 0; --i) {
		j = randint0(i);

		max1 = p->stat_max[i];
		cur1 = p->stat_cur[i];
		max2 = p->stat_max[j];
		cur2 = p->stat_cur[j];

		p->stat_max[i] = max2;
		p->stat_cur[i] = cur2;
		p->stat_max[j] = max1;
		p->stat_cur[j] = cur1;

		/* Record what we did */
		swap = p->stat_map[i];
		assert(swap >= 0 && swap < STAT_MAX);
		p->stat_map[i] = p->stat_map[j];
		assert(p->stat_map[i] >= 0 && p->stat_map[i] < STAT_MAX);
		p->stat_map[j] = swap;
	}

	/* Mark what else needs to be updated */
	p->upkeep->update |= (PU_BONUS);
}

/**
 * Revert all prior swaps to the player's stats.  Has no effect if the
 * stats have not been swapped.
 */
void player_fix_scramble(struct player *p)
{
	/* Figure out what stats should be */
	int new_cur[STAT_MAX];
	int new_max[STAT_MAX];
	int i;

	for (i = 0; i < STAT_MAX; ++i) {
		assert(p->stat_map[i] >= 0 && p->stat_map[i] < STAT_MAX);
		new_cur[p->stat_map[i]] = p->stat_cur[i];
		new_max[p->stat_map[i]] = p->stat_max[i];
	}

	/* Apply new stats and reset stat_map */
	for (i = 0; i < STAT_MAX; ++i) {
		p->stat_cur[i] = new_cur[i];
		p->stat_max[i] = new_max[i];
		p->stat_map[i] = i;
	}

	/* Mark what else needs to be updated */
	p->upkeep->update |= (PU_BONUS);
}

/**
 * Regenerate one turn's worth of hit points
 */
void player_regen_hp(struct player *p)
{
	int32_t hp_gain;
	int percent = 0;/* max 32k -> 50% of mhp; more accurately "pertwobytes" */
	int fed_pct, old_chp = p->chp;

	/* Default regeneration */
	if (p->timed[TMD_FOOD] >= PY_FOOD_WEAK) {
		percent = PY_REGEN_NORMAL;
	} else if (p->timed[TMD_FOOD] >= PY_FOOD_FAINT) {
		percent = PY_REGEN_WEAK;
	} else if (p->timed[TMD_FOOD] >= PY_FOOD_STARVE) {
		percent = PY_REGEN_FAINT;
	}

	/* Food bonus - better fed players regenerate up to 1/3 faster */
	fed_pct = p->timed[TMD_FOOD] / z_info->food_value;
	percent *= 100 + fed_pct / 3;
	percent /= 100;

	/* Various things speed up regeneration */
	if (player_of_has(p, OF_REGEN))
		percent *= 2;
	if (player_resting_can_regenerate(p))
		percent *= 2;

	/* Some things slow it down */
	if (player_of_has(p, OF_IMPAIR_HP))
		percent /= 2;

	/* Various things interfere with physical healing */
	if (p->timed[TMD_PARALYZED]) percent = 0;
	if (p->timed[TMD_POISONED]) percent = 0;
	if (p->timed[TMD_STUN]) percent = 0;
	if (p->timed[TMD_CUT]) percent = 0;

	/* Extract the new hitpoints */
	hp_gain = (int32_t)(p->mhp * percent) + PY_REGEN_HPBASE;
	player_adjust_hp_precise(p, hp_gain);

	/* Notice changes */
	if (old_chp != p->chp) {
		equip_learn_flag(p, OF_REGEN);
		equip_learn_flag(p, OF_IMPAIR_HP);
	}
}


/**
 * Regenerate one turn's worth of mana
 */
void player_regen_mana(struct player *p)
{
	int32_t sp_gain;
	int percent, old_csp = p->csp;

	/* Save the old spell points */
	old_csp = p->csp;

	/* Default regeneration */
	percent = PY_REGEN_NORMAL;

	/* Various things speed up regeneration, but shouldn't punish healthy BGs */
	if (!(player_has(p, PF_COMBAT_REGEN) && p->chp  > p->mhp / 2)) {
		if (player_of_has(p, OF_REGEN))
			percent *= 2;
		if (player_resting_can_regenerate(p))
			percent *= 2;
	}

	/* Some things slow it down */
	if (player_has(p, PF_COMBAT_REGEN)) {
		percent /= -2;
	} else if (player_of_has(p, OF_IMPAIR_MANA)) {
		percent /= 2;
	}

	/* Regenerate mana */
	sp_gain = (int32_t)(p->msp * percent);
	if (percent >= 0)
		sp_gain += PY_REGEN_MNBASE;
	sp_gain = player_adjust_mana_precise(p, sp_gain);

	/* SP degen heals BGs at double efficiency vs casting */
	if (sp_gain < 0  && player_has(p, PF_COMBAT_REGEN)) {
		convert_mana_to_hp(p, -sp_gain * 2);
	}

	/* Notice changes */
	if (old_csp != p->csp) {
		p->upkeep->redraw |= (PR_MANA);
		equip_learn_flag(p, OF_REGEN);
		equip_learn_flag(p, OF_IMPAIR_MANA);
	}
}

void player_adjust_hp_precise(struct player *p, int32_t hp_gain)
{
	int16_t old_16 = p->chp;
	/* Load it all into 4 byte format */
	int32_t old_32 = ((int32_t) old_16) * 65536 + p->chp_frac, new_32;

	/* Check for overflow */
	if (hp_gain >= 0) {
		new_32 = (old_32 < INT32_MAX - hp_gain) ?
			old_32 + hp_gain : INT32_MAX;
	} else {
		new_32 = (old_32 > INT32_MIN - hp_gain) ?
			old_32 + hp_gain : INT32_MIN;
	}

	/* Break it back down */
	if (new_32 < 0) {
		/*
		 * Don't use right bitwise shift on negative values:  whether
		 * the left bits are zero or one depends on the system.
		 */
		int32_t remainder = new_32 % 65536;

		p->chp = (int16_t) (new_32 / 65536);
		if (remainder) {
			assert(remainder < 0);
			p->chp_frac = (uint16_t) (65536 + remainder);
			assert(p->chp > INT16_MIN);
			p->chp -= 1;
		} else {
			p->chp_frac = 0;
		}
	} else {
		p->chp = (int16_t)(new_32 >> 16);   /* div 65536 */
		p->chp_frac = (uint16_t)(new_32 & 0xFFFF); /* mod 65536 */
	}

	/* Fully healed */
	if (p->chp >= p->mhp) {
		p->chp = p->mhp;
		p->chp_frac = 0;
	}

	if (p->chp != old_16) {
		p->upkeep->redraw |= (PR_HP);
	}
}


/**
 * Accept a 4 byte signed int, divide it by 65k, and add
 * to current spell points. p->csp and csp_frac are 2 bytes each.
 */
int32_t player_adjust_mana_precise(struct player *p, int32_t sp_gain)
{
	int16_t old_16 = p->csp;
	/* Load it all into 4 byte format*/
	int32_t old_32 = ((int32_t) p->csp) * 65536 + p->csp_frac, new_32;

	if (sp_gain == 0) return 0;

	/* Check for overflow */
	if (sp_gain > 0) {
		if (old_32 < INT32_MAX - sp_gain) {
			new_32 = old_32 + sp_gain;
		} else {
			new_32 = INT32_MAX;
			sp_gain = 0;
		}
	} else if (old_32 > INT32_MIN - sp_gain) {
		new_32 = old_32 + sp_gain;
	} else {
		new_32 = INT32_MIN;
		sp_gain = 0;
	}

	/* Break it back down*/
	if (new_32 < 0) {
		/*
		 * Don't use right bitwise shift on negative values:  whether
		 * the left bits are zero or one depends on the system.
		 */
		int32_t remainder = new_32 % 65536;

		p->csp = (int16_t) (new_32 / 65536);
		if (remainder) {
			assert(remainder < 0);
			p->csp_frac = (uint16_t) (65536 + remainder);
			assert(p->csp > INT16_MIN);
			p->csp -= 1;
		} else {
			p->chp_frac = 0;
		}
	} else {
		p->csp = (int16_t)(new_32 >> 16);   /* div 65536 */
		p->csp_frac = (uint16_t)(new_32 & 0xFFFF);    /* mod 65536 */
	}

	/* Max/min SP */
	if (p->csp >= p->msp) {
		p->csp = p->msp;
		p->csp_frac = 0;
		sp_gain = 0;
	} else if (p->csp < 0) {
		p->csp = 0;
		p->csp_frac = 0;
		sp_gain = 0;
	}

	/* Notice changes */
	if (old_16 != p->csp) {
		p->upkeep->redraw |= (PR_MANA);
	}

	if (sp_gain == 0) {
		/* Recalculate */
		new_32 = ((int32_t) p->csp) * 65536 + p->csp_frac;
		sp_gain = new_32 - old_32;
	}

	return sp_gain;
}

void convert_mana_to_hp(struct player *p, int32_t sp_long) {
	int32_t hp_gain, sp_ratio;

	if (sp_long <= 0 || p->msp == 0 || p->mhp == p->chp) return;

	/* Total HP from max */
	hp_gain = ((int32_t)(p->mhp - p->chp)) * 65536;
	hp_gain -= (int32_t)p->chp_frac;

	/* Spend X% of SP get X/2% of lost HP. E.g., at 50% HP get X/4% */
	/* Gain stays low at msp<10 because MP gains are generous at msp<10 */
	/* sp_ratio is max sp to spent sp, doubled to suit target rate. */
	sp_ratio = (((int32_t)MAX(10, (int32_t)p->msp)) * 131072) / sp_long;

	/* Limit max healing to 25% of damage; ergo spending > 50% msp
	 * is inefficient */
	if (sp_ratio < 4) {sp_ratio = 4;}
	hp_gain /= sp_ratio;

	/* DAVIDTODO Flavorful comments on large gains would be fun and informative */

	player_adjust_hp_precise(p, hp_gain);
}

/**
 * Update the player's light fuel
 */
void player_update_light(struct player *p)
{
	/* Check for light being wielded */
	struct object *obj = equipped_item_by_slot_name(p, "light");

	/* Burn some fuel in the current light */
	if (obj && tval_is_light(obj)) {
		bool burn_fuel = true;

		/* Turn off the wanton burning of light during the day in the town */
		if (!p->depth && is_daytime())
			burn_fuel = false;

		/* If the light has the NO_FUEL flag, well... */
		if (of_has(obj->flags, OF_NO_FUEL))
		    burn_fuel = false;

		/* Use some fuel (except on artifacts, or during the day) */
		if (burn_fuel && obj->timeout > 0) {
			/* Decrease life-span */
			obj->timeout--;

			/* Hack -- notice interesting fuel steps */
			if ((obj->timeout < 100) || (!(obj->timeout % 100)))
				/* Redraw stuff */
				p->upkeep->redraw |= (PR_EQUIP);

			/* Hack -- Special treatment when blind */
			if (p->timed[TMD_BLIND]) {
				/* Hack -- save some light for later */
				if (obj->timeout == 0) obj->timeout++;
			} else if (obj->timeout == 0) {
				/* The light is now out */
				disturb(p);
				msg("Your light has gone out!");

				/* If it's a torch, now is the time to delete it */
				if (of_has(obj->flags, OF_BURNS_OUT)) {
					bool dummy;
					struct object *burnt =
						gear_object_for_use(p, obj, 1,
						false, &dummy);
					if (burnt->known)
						object_delete(p->cave, NULL, &burnt->known);
					object_delete(cave, p->cave, &burnt);
				}
			} else if ((obj->timeout < 50) && (!(obj->timeout % 20))) {
				/* The light is getting dim */
				disturb(p);
				msg("Your light is growing faint.");
			}
		}
	}

	/* Calculate torch radius */
	p->upkeep->update |= (PU_TORCH);
}

/**
 * Find the player's best digging tool.  If forbid_stack is true, ignores
 * stacks of more than one item.
 */
struct object *player_best_digger(struct player *p, bool forbid_stack)
{
	int weapon_slot = slot_by_name(p, "weapon");
	struct object *current_weapon = slot_object(p, weapon_slot);
	struct object *obj, *best = NULL;
	/* Prefer any melee weapon over unarmed digging, i.e. best == NULL. */
	int best_score = -1;
	struct player_state local_state;

	for (obj = p->gear; obj; obj = obj->next) {
		int score, old_number;
		if (!tval_is_melee_weapon(obj)) continue;
		if (obj->number < 1 || (forbid_stack && obj->number > 1)) continue;
		/* Don't use it if it has a sticky curse. */
		if (!obj_can_takeoff(obj)) continue;

		/* Swap temporarily for the calc_bonuses() computation. */
		old_number = obj->number;
		if (obj != current_weapon) {
			obj->number = 1;
			p->body.slots[weapon_slot].obj = obj;
		}

		/*
		 * Avoid side effects from using update set to false
		 * with calc_bonuses().
		 */
		local_state.stat_ind[STAT_STR] = 0;
		local_state.stat_ind[STAT_DEX] = 0;
		calc_bonuses(p, &local_state, true, false);
		score = local_state.skills[SKILL_DIGGING];

		/* Swap back. */
		if (obj != current_weapon) {
			obj->number = old_number;
			p->body.slots[weapon_slot].obj = current_weapon;
		}

		if (score > best_score) {
			best = obj;
			best_score = score;
		}
	}

	return best;
}

/**
 * Melee a random adjacent monster
 */
bool player_attack_random_monster(struct player *p)
{
	int i, dir = randint0(8);

	/* Confused players get a free pass */
	if (p->timed[TMD_CONFUSED]) return false;

	/* Look for a monster, attack */
	for (i = 0; i < 8; i++, dir++) {
		struct loc grid = loc_sum(p->grid, ddgrid_ddd[dir % 8]);
		const struct monster *mon = square_monster(cave, grid);
		if (mon && !monster_is_camouflaged(mon)) {
			p->upkeep->energy_use = z_info->move_energy;
			msg("You angrily lash out at a nearby foe!");
			py_attack(p, grid);
			return true;
		}
	}
	return false;
}

/**
 * Have random bad stuff happen to the player from over-exertion
 *
 * This function uses the PY_EXERT_* flags
 */
void player_over_exert(struct player *p, int flag, int chance, int amount)
{
	if (chance <= 0) return;

	/* CON damage */
	if (flag & PY_EXERT_CON) {
		if (randint0(100) < chance) {
			/* Hack - only permanent with high chance (no-mana casting) */
			bool perm = (randint0(100) < chance / 2) && (chance >= 50);
			msg("You have damaged your health!");
			player_stat_dec(p, STAT_CON, perm);
		}
	}

	/* Fainting */
	if (flag & PY_EXERT_FAINT) {
		if (randint0(100) < chance) {
			msg("You faint from the effort!");

			/* Bypass free action */
			(void)player_inc_timed(p, TMD_PARALYZED,
				randint1(amount), true, true, false);
		}
	}

	/* Scrambled stats */
	if (flag & PY_EXERT_SCRAMBLE) {
		if (randint0(100) < chance) {
			(void)player_inc_timed(p, TMD_SCRAMBLE,
				randint1(amount), true, true, true);
		}
	}

	/* Cut damage */
	if (flag & PY_EXERT_CUT) {
		if (randint0(100) < chance) {
			msg("Wounds appear on your body!");
			(void)player_inc_timed(p, TMD_CUT, randint1(amount),
				true, true, false);
		}
	}

	/* Confusion */
	if (flag & PY_EXERT_CONF) {
		if (randint0(100) < chance) {
			(void)player_inc_timed(p, TMD_CONFUSED,
				randint1(amount), true, true, true);
		}
	}

	/* Hallucination */
	if (flag & PY_EXERT_HALLU) {
		if (randint0(100) < chance) {
			(void)player_inc_timed(p, TMD_IMAGE, randint1(amount),
				true, true, true);
		}
	}

	/* Slowing */
	if (flag & PY_EXERT_SLOW) {
		if (randint0(100) < chance) {
			msg("You feel suddenly lethargic.");
			(void)player_inc_timed(p, TMD_SLOW, randint1(amount),
				true, true, false);
		}
	}

	/* HP */
	if (flag & PY_EXERT_HP) {
		if (randint0(100) < chance) {
			msg("You cry out in sudden pain!");
			take_hit(p, randint1(amount), "over-exertion");
		}
	}
}


/**
 * See how much damage the player will take from terrain.
 *
 * \param p is the player to check
 * \param grid is the location of the terrain
 * \param actual, if true, will cause the player to learn the appropriate
 * runes if equipment or effects mitigate the damage.
 */
int player_check_terrain_damage(struct player *p, struct loc grid, bool actual)
{
	int dam_taken = 0;

	if (square_isfiery(cave, grid)) {
		int base_dam = 100 + randint1(100);
		int res = p->state.el_info[ELEM_FIRE].res_level;

		/* Fire damage */
		dam_taken = adjust_dam(p, ELEM_FIRE, base_dam, RANDOMISE, res,
			actual);

		/* Feather fall makes one lightfooted. */
		if (player_of_has(p, OF_FEATHER)) {
			dam_taken /= 2;
			if (actual) {
				equip_learn_flag(p, OF_FEATHER);
			}
		}
	}

	return dam_taken;
}

/**
 * Terrain damages the player
 */
void player_take_terrain_damage(struct player *p, struct loc grid)
{
	int dam_taken = player_check_terrain_damage(p, grid, true);

	if (!dam_taken) {
		return;
	}

	/* Damage the player and inventory */
	if (square_isfiery(cave, grid)) {
		msg(square_feat(cave, grid)->hurt_msg);
		inven_damage(p, PROJ_FIRE, dam_taken);
	}
	take_hit(p, dam_taken, square_feat(cave, grid)->die_msg);
}

/**
 * Find a player shape from the name
 */
struct player_shape *lookup_player_shape(const char *name)
{
	struct player_shape *shape = shapes;
	while (shape) {
		if (streq(shape->name, name)) {
			return shape;
		}
		shape = shape->next;
	}
	msg("Could not find %s shape!", name);
	return NULL;
}

/**
 * Find a player shape index from the shape name
 */
int shape_name_to_idx(const char *name)
{
	struct player_shape *shape = lookup_player_shape(name);
	if (shape) {
		return shape->sidx;
	} else {
		return -1;
	}
}

/**
 * Find a player shape from the index
 */
struct player_shape *player_shape_by_idx(int index)
{
	struct player_shape *shape = shapes;
	while (shape) {
		if (shape->sidx == index) {
			return shape;
		}
		shape = shape->next;
	}
	msg("Could not find shape %d!", index);
	return NULL;
}

/**
 * Give shapechanged players a choice of returning to normal shape and
 * performing a command, just returning to normal shape without acting, or
 * canceling.
 *
 * \param p the player
 * \param cmd the command being performed
 * \return true if the player wants to proceed with their command
 */
bool player_get_resume_normal_shape(struct player *p, struct command *cmd)
{
	if (player_is_shapechanged(p)) {
		msg("You cannot do this while in %s form.", p->shape->name);
		char prompt[100];
		strnfmt(prompt, sizeof(prompt),
		        "Change back and %s (y/n) or (r)eturn to normal? ",
		        cmd_verb(cmd->code));
		char answer = get_char(prompt, "yrn", 3, 'n');

		// Change back to normal shape
		if (answer == 'y' || answer == 'r') {
			player_resume_normal_shape(p);
		}

		// Players may only act if they return to normal shape
		return answer == 'y';
	}

	// Normal shape players can proceed as usual
	return true;
}

/**
 * Revert to normal shape
 */
void player_resume_normal_shape(struct player *p)
{
	p->shape = lookup_player_shape("normal");
	msg("You resume your usual shape.");

	/* Kill vampire attack */
	(void) player_clear_timed(p, TMD_ATT_VAMP, true, false);

	/* Update */
	p->upkeep->update |= (PU_BONUS);
	p->upkeep->redraw |= (PR_TITLE | PR_MISC);
	handle_stuff(p);
}

/**
 * Check if the player is shapechanged
 */
bool player_is_shapechanged(const struct player *p)
{
	return streq(p->shape->name, "normal") ? false : true;
}

/**
 * Check if the player is immune from traps
 */
bool player_is_trapsafe(const struct player *p)
{
	if (p->timed[TMD_TRAPSAFE]) return true;
	if (player_of_has(p, OF_TRAP_IMMUNE)) return true;
	return false;
}

/**
 * Return true if the player can cast a spell.
 *
 * \param p is the player
 * \param show_msg should be set to true if a failure message should be
 * displayed.
 */
bool player_can_cast(const struct player *p, bool show_msg)
{
	if (!p->class->magic.total_spells) {
		if (show_msg) {
			msg("You cannot pray or produce magics.");
		}
		return false;
	}

	if (p->timed[TMD_BLIND] || no_light(p)) {
		if (show_msg) {
			msg("You cannot see!");
		}
		return false;
	}

	if (p->timed[TMD_CONFUSED]) {
		if (show_msg) {
			msg("You are too confused!");
		}
		return false;
	}

	return true;
}

/**
 * Return true if the player can study a spell.
 *
 * \param p is the player
 * \param show_msg should be set to true if a failure message should be
 * displayed.
 */
bool player_can_study(const struct player *p, bool show_msg)
{
	if (!player_can_cast(p, show_msg))
		return false;

	if (!p->upkeep->new_spells) {
		if (show_msg) {
			int count;
			struct magic_realm *r = class_magic_realms(p->class, &count), *r1;
			char buf[120];

			my_strcpy(buf, r->spell_noun, sizeof(buf));
			my_strcat(buf, "s", sizeof(buf));
			r1 = r->next;
			mem_free(r);
			r = r1;
			if (count > 1) {
				while (r) {
					count--;
					if (count) {
						my_strcat(buf, ", ", sizeof(buf));
					} else {
						my_strcat(buf, " or ", sizeof(buf));
					}
					my_strcat(buf, r->spell_noun, sizeof(buf));
					my_strcat(buf, "s", sizeof(buf));
					r1 = r->next;
					mem_free(r);
					r = r1;
				}
			}
			msg("You cannot learn any new %s!", buf);
		}
		return false;
	}

	return true;
}

/**
 * Return true if the player can read scrolls or books.
 *
 * \param p is the player
 * \param show_msg should be set to true if a failure message should be
 * displayed.
 */
bool player_can_read(const struct player *p, bool show_msg)
{
	if (p->timed[TMD_BLIND]) {
		if (show_msg)
			msg("You can't see anything.");

		return false;
	}

	if (no_light(p)) {
		if (show_msg)
			msg("You have no light to read by.");

		return false;
	}

	if (p->timed[TMD_CONFUSED]) {
		if (show_msg)
			msg("You are too confused to read!");

		return false;
	}

	if (p->timed[TMD_AMNESIA]) {
		if (show_msg)
			msg("You can't remember how to read!");

		return false;
	}

	return true;
}

/**
 * Return true if the player can fire something with a launcher.
 *
 * \param p is the player
 * \param show_msg should be set to true if a failure message should be
 * displayed.
 */
bool player_can_fire(struct player *p, bool show_msg)
{
	struct object *obj = equipped_item_by_slot_name(p, "shooting");

	/* Require a usable launcher */
	if (!obj || !p->state.ammo_tval) {
		if (show_msg)
			msg("You have nothing to fire with.");
		return false;
	}

	return true;
}

/**
 * Return true if the player can refuel their light source.
 *
 * \param p is the player
 * \param show_msg should be set to true if a failure message should be
 * displayed.
 */
bool player_can_refuel(struct player *p, bool show_msg)
{
	struct object *obj = equipped_item_by_slot_name(p, "light");

	if (obj && of_has(obj->flags, OF_TAKES_FUEL)) {
		return true;
	}

	if (show_msg) {
		msg("Your light cannot be refuelled.");
	}

	return false;
}

/**
 * Prerequisite function for command. See struct cmd_info in ui-input.h and
 * it's use in ui-game.c.
 */
bool player_can_cast_prereq(void)
{
	return player_can_cast(player, true);
}

/**
 * Prerequisite function for command. See struct cmd_info in ui-input.h and
 * it's use in ui-game.c.
 */
bool player_can_study_prereq(void)
{
	return player_can_study(player, true);
}

/**
 * Prerequisite function for command. See struct cmd_info in ui-input.h and
 * it's use in ui-game.c.
 */
bool player_can_read_prereq(void)
{
	/*
	 * Accommodate hacks elsewhere:  'r' is overloaded to mean
	 * release a commanded monster when TMD_COMMAND is active.
	 */
	return (player->timed[TMD_COMMAND]) ?
		true : player_can_read(player, true);
}

/**
 * Prerequisite function for command. See struct cmd_info in ui-input.h and
 * it's use in ui-game.c.
 */
bool player_can_fire_prereq(void)
{
	return player_can_fire(player, true);
}

/**
 * Prerequisite function for command. See struct cmd_info in ui-input.h and
 * it's use in ui-game.c.
 */
bool player_can_refuel_prereq(void)
{
	return player_can_refuel(player, true);
}

/**
 * Prerequisite function for command. See struct cmd_info in ui-input.h and
 * it's use in ui-game.c.
 */
bool player_can_debug_prereq(void)
{
	if (player->noscore & NOSCORE_DEBUG) {
		return true;
	}
	if (confirm_debug()) {
		/* Mark savefile */
		player->noscore |= NOSCORE_DEBUG;
		return true;
	}
	return false;
}


/**
 * Return true if the player has access to a book that has unlearned spells.
 *
 * \param p is the player
 */
bool player_book_has_unlearned_spells(struct player *p)
{
	int i, j;
	int item_max = z_info->pack_size + z_info->floor_size;
	struct object **item_list = mem_zalloc(item_max * sizeof(struct object *));
	int item_num;

	/* Check if the player can learn new spells */
	if (!p->upkeep->new_spells) {
		mem_free(item_list);
		return false;
	}

	/* Check through all available books */
	item_num = scan_items(item_list, item_max, p, USE_INVEN | USE_FLOOR,
		obj_can_study);
	for (i = 0; i < item_num; i++) {
		const struct class_book *book = player_object_to_book(p, item_list[i]);
		if (!book) continue;

		/* Extract spells */
		for (j = 0; j < book->num_spells; j++)
			if (spell_okay_to_study(p, book->spells[j].sidx)) {
				/* There is a spell the player can study */
				mem_free(item_list);
				return true;
			}
	}

	mem_free(item_list);
	return false;
}

/**
 * Apply confusion, if needed, to a direction
 *
 * Display a message and return true if direction changes.
 */
bool player_confuse_dir(struct player *p, int *dp, bool too)
{
	int dir = *dp;

	if (p->timed[TMD_CONFUSED]) {
		if ((dir == 5) || (randint0(100) < 75)) {
			/* Random direction */
			dir = ddd[randint0(8)];
		}

	/* Running attempts always fail */
	if (too) {
		msg("You are too confused.");
		return true;
	}

	if (*dp != dir) {
		msg("You are confused.");
		*dp = dir;
		return true;
	}
	}

	return false;
}

/**
 * Return true if the provided count is one of the conditional REST_ flags.
 */
bool player_resting_is_special(int16_t count)
{
	switch (count) {
		case REST_COMPLETE:
		case REST_ALL_POINTS:
		case REST_SOME_POINTS:
			return true;
	}

	return false;
}

/**
 * Return true if the player is resting.
 */
bool player_is_resting(const struct player *p)
{
	return (p->upkeep->resting > 0 ||
			player_resting_is_special(p->upkeep->resting));
}

/**
 * Return the remaining number of resting turns.
 */
int16_t player_resting_count(const struct player *p)
{
	return p->upkeep->resting;
}

/**
 * In order to prevent the regeneration bonus from the first few turns, we have
 * to store the number of turns the player has rested. Otherwise, the first
 * few turns will have the bonus and the last few will not.
 */
static int player_turns_rested = 0;
static bool player_rest_disturb = false;

/**
 * Set the number of resting turns.
 *
 * \param count is the number of turns to rest or one of the REST_ constants.
 */
void player_resting_set_count(struct player *p, int16_t count)
{
	/* Cancel if player is disturbed */
	if (player_rest_disturb) {
		p->upkeep->resting = 0;
		player_rest_disturb = false;
		return;
	}

	/* Ignore if the rest count is negative. */
	if ((count < 0) && !player_resting_is_special(count)) {
		p->upkeep->resting = 0;
		return;
	}

	/* Save the rest code */
	p->upkeep->resting = count;

	/* Truncate overlarge values */
	if (p->upkeep->resting > 9999) p->upkeep->resting = 9999;
}

/**
 * Cancel current rest.
 */
void player_resting_cancel(struct player *p, bool disturb)
{
	player_resting_set_count(p, 0);
	player_turns_rested = 0;
	player_rest_disturb = disturb;
}

/**
 * Return true if the player should get a regeneration bonus for the current
 * rest.
 */
bool player_resting_can_regenerate(const struct player *p)
{
	return player_turns_rested >= REST_REQUIRED_FOR_REGEN ||
		player_resting_is_special(p->upkeep->resting);
}

/**
 * Perform one turn of resting. This only handles the bookkeeping of resting
 * itself, and does not calculate any possible other effects of resting (see
 * process_world() for regeneration).
 */
void player_resting_step_turn(struct player *p)
{
	/* Timed rest */
	if (p->upkeep->resting > 0) {
		/* Reduce rest count */
		p->upkeep->resting--;

		/* Redraw the state */
		p->upkeep->redraw |= (PR_STATE);
	}

	/* Take a turn */
	p->upkeep->energy_use = z_info->move_energy;

	/* Increment the resting counters */
	p->resting_turn++;
	player_turns_rested++;
}

/**
 * Handle the conditions for conditional resting (resting with the REST_
 * constants).
 */
void player_resting_complete_special(struct player *p)
{
	/* Complete resting */
	if (!player_resting_is_special(p->upkeep->resting)) return;

	if (p->upkeep->resting == REST_ALL_POINTS) {
		if ((p->chp == p->mhp) && (p->csp == p->msp))
			/* Stop resting */
			disturb(p);
	} else if (p->upkeep->resting == REST_COMPLETE) {
		if ((p->chp == p->mhp) &&
			(p->csp == p->msp || player_has(p, PF_COMBAT_REGEN)) &&
			!p->timed[TMD_BLIND] && !p->timed[TMD_CONFUSED] &&
			!p->timed[TMD_POISONED] && !p->timed[TMD_AFRAID] &&
			!p->timed[TMD_TERROR] && !p->timed[TMD_STUN] &&
			!p->timed[TMD_CUT] && !p->timed[TMD_SLOW] &&
			!p->timed[TMD_PARALYZED] && !p->timed[TMD_IMAGE] &&
			!p->word_recall && !p->deep_descent)
			/* Stop resting */
			disturb(p);
	} else if (p->upkeep->resting == REST_SOME_POINTS) {
		if ((p->chp == p->mhp) || (p->csp == p->msp))
			/* Stop resting */
			disturb(p);
	}
}

/* Record the player's last rest count for repeating */
static int player_resting_repeat_count = 0;

/**
 * Get the number of resting turns to repeat.
 *
 * \param p The current player.
 */
int player_get_resting_repeat_count(struct player *p)
{
	return player_resting_repeat_count;
}

/**
 * Set the number of resting turns to repeat.
 *
 * \param count is the number of turns requested for rest most recently.
 */
void player_set_resting_repeat_count(struct player *p, int16_t count)
{
	player_resting_repeat_count = count;
}

/**
 * Check if the player state has the given OF_ flag.
 */
bool player_of_has(const struct player *p, int flag)
{
	assert(p);
	return of_has(p->state.flags, flag);
}

/**
 * Check if the player resists (or better) an element
 */
bool player_resists(const struct player *p, int element)
{
	return (p->state.el_info[element].res_level > 0);
}

/**
 * Check if the player resists (or better) an element
 */
bool player_is_immune(const struct player *p, int element)
{
	return (p->state.el_info[element].res_level == 3);
}

/**
 * Places the player at the given coordinates in the cave.
 */
void player_place(struct chunk *c, struct player *p, struct loc grid)
{
	assert(!square_monster(c, grid));

	/* Save player location */
	p->grid = grid;

	/* Mark cave grid */
	square_set_mon(c, grid, -1);

	/* Clear stair creation */
	p->upkeep->create_down_stair = false;
	p->upkeep->create_up_stair = false;
}

/*
 * Take care of bookkeeping after moving the player with monster_swap().
 *
 * \param p is the player that was moved.
 * \param eval_trap, if true, will cause evaluation (possibly affecting the
 * player) of the traps in the grid.
 * \param is_involuntary, if true, will do appropriate actions (flush the
 * command queue) for a move not expected by the player.
 */
void player_handle_post_move(struct player *p, bool eval_trap,
		bool is_involuntary)
{
	/* Handle store doors, or notice objects */
	if (square_isshop(cave, p->grid)) {
		if (player_is_shapechanged(p)) {
			if (square(cave, p->grid)->feat != FEAT_HOME) {
				msg("There is a scream and the door slams shut!");
			}
			return;
		}
		disturb(p);
		if (is_involuntary) {
			cmdq_flush();
		}
		event_signal(EVENT_ENTER_STORE);
		event_remove_handler_type(EVENT_ENTER_STORE);
		event_signal(EVENT_USE_STORE);
		event_remove_handler_type(EVENT_USE_STORE);
		event_signal(EVENT_LEAVE_STORE);
		event_remove_handler_type(EVENT_LEAVE_STORE);
	} else {
		if (is_involuntary) {
			cmdq_flush();
		}
		square_know_pile(cave, p->grid);
	}

	/* Discover invisible traps, set off visible ones */
	if (eval_trap && square_isplayertrap(cave, p->grid)
			&& !square_isdisabledtrap(cave, p->grid)) {
		hit_trap(p->grid, 0);
	}

	/* Update view and search */
	update_view(cave, p);
	search(p);
}

/*
 * Something has happened to disturb the player.
 *
 * All disturbance cancels repeated commands, resting, and running.
 *
 * XXX-AS: Make callers either pass in a command
 * or call cmd_cancel_repeat inside the function calling this
 */
void disturb(struct player *p)
{
	/* Cancel repeated commands */
	cmd_cancel_repeat();

	/* Cancel Resting */
	if (player_is_resting(p)) {
		player_resting_cancel(p, true);
		p->upkeep->redraw |= PR_STATE;
	}

	/* Cancel running */
	if (p->upkeep->running) {
		p->upkeep->running = 0;

		/* Cancel queued commands */
		cmdq_flush();

		/* Check for new panel if appropriate */
		event_signal(EVENT_PLAYERMOVED);
		p->upkeep->update |= PU_TORCH;

		/* Mark the whole map to be redrawn */
		event_signal_point(EVENT_MAP, -1, -1);
	}

	/* Flush input */
	event_signal(EVENT_INPUT_FLUSH);
}

/**
 * Search for traps or secret doors
 */
void search(struct player *p)
{
	struct loc grid;

	/* Various conditions mean no searching */
	if (p->timed[TMD_BLIND] || no_light(p) ||
		p->timed[TMD_CONFUSED] || p->timed[TMD_IMAGE])
		return;

	/* Search the nearby grids, which are always in bounds */
	for (grid.y = (p->grid.y - 1); grid.y <= (p->grid.y + 1); grid.y++) {
		for (grid.x = (p->grid.x - 1); grid.x <= (p->grid.x + 1); grid.x++) {
			struct object *obj;

			/* Secret doors */
			if (square_issecretdoor(cave, grid)) {
				msg("You have found a secret door.");
				place_closed_door(cave, grid);
				disturb(p);
			}

			/* Traps on chests */
			for (obj = square_object(cave, grid); obj; obj = obj->next) {
				if (!obj->known || ignore_item_ok(p, obj)
						|| !is_trapped_chest(obj)) {
					continue;
				}

				if (obj->known->pval != obj->pval) {
					msg("You have discovered a trap on the chest!");
					obj->known->pval = obj->pval;
					disturb(p);
				}
			}
		}
	}
}
