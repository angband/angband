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
#include "obj-knowledge.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-calcs.h"
#include "player-history.h"
#include "player-spell.h"
#include "player-timed.h"
#include "player-util.h"
#include "project.h"
#include "score.h"
#include "store.h"
#include "target.h"
#include "trap.h"

/**
 * Increment to the next or decrement to the preceeding level
   accounting for the stair skip value in constants
   Keep in mind to check all intermediate level for unskippable
   quests
*/
int dungeon_get_next_level(int dlev, int added)
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
		if (is_quest(i)) return i;
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
		if ((p->max_depth < z_info->max_depth - 1) && !is_quest(p->max_depth)) {
			p->recall_depth = dungeon_get_next_level(p->max_depth, 1);
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
		char *prompt = "Which level do you wish to return to (0 to cancel)? ";
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
	if (dam <= 0) return;

	/* Disturb */
	disturb(p, 1);

	/* Hurt the player */
	p->chp -= dam;

	/* Display the hitpoints */
	p->upkeep->redraw |= (PR_HP);

	/* Dead player */
	if (p->chp < 0) {
		/* Allow cheating */
		if ((p->wizard || OPT(p, cheat_live)) && !get_check("Die? ")) {
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
			bell("Low hitpoint warning!");

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
	struct store *home = &stores[STORE_HOME];
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
		object_flavor_aware(obj);
		obj->known->effect = obj->effect;
		obj->known->activation = obj->activation;
	}

	for (obj = home->stock; obj; obj = obj->next) {
		object_flavor_aware(obj);
		obj->known->effect = obj->effect;
		obj->known->activation = obj->activation;
	}

	history_unmask_unknown(p);

	/* Get time of death */
	(void)time(&death_time);
	enter_score(&death_time);

	/* Hack -- Recalculate bonuses */
	p->upkeep->update |= (PU_BONUS);
	handle_stuff(p);
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
s16b modify_stat_value(int value, int amount)
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
 * Regenerate hit points
 */
void player_regen_hp(struct player *p)
{
	s32b new_chp, new_chp_frac;
	int old_chp, percent = 0;

	/* Save the old hitpoints */
	old_chp = p->chp;

	/* Default regeneration */
	if (p->food >= PY_FOOD_WEAK)
		percent = PY_REGEN_NORMAL;
	else if (p->food >= PY_FOOD_FAINT)
		percent = PY_REGEN_WEAK;
	else if (p->food >= PY_FOOD_STARVE)
		percent = PY_REGEN_FAINT;

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
	new_chp = ((long)p->mhp) * percent + PY_REGEN_HPBASE;
	p->chp += (s16b)(new_chp >> 16);   /* div 65536 */

	/* Check for overflow */
	if ((p->chp < 0) && (old_chp > 0))
		p->chp = SHRT_MAX;
	new_chp_frac = (new_chp & 0xFFFF) + p->chp_frac;	/* mod 65536 */
	if (new_chp_frac >= 0x10000L) {
		p->chp_frac = (u16b)(new_chp_frac - 0x10000L);
		p->chp++;
	} else {
		p->chp_frac = (u16b)new_chp_frac;
	}

	/* Fully healed */
	if (p->chp >= p->mhp) {
		p->chp = p->mhp;
		p->chp_frac = 0;
	}

	/* Notice changes */
	if (old_chp != p->chp) {
		p->upkeep->redraw |= (PR_HP);
		equip_learn_flag(p, OF_REGEN);
		equip_learn_flag(p, OF_IMPAIR_HP);
	}
}


/**
 * Regenerate mana points
 */
void player_regen_mana(struct player *p)
{
	s32b new_mana, new_mana_frac;
	int old_csp, percent;

	/* Save the old spell points */
	old_csp = p->csp;

	/* Default regeneration */
	percent = PY_REGEN_NORMAL;

	/* Various things speed up regeneration */
	if (player_of_has(p, OF_REGEN))
		percent *= 2;
	if (player_resting_can_regenerate(p))
		percent *= 2;

	/* Some things slow it down */
	if (player_of_has(p, OF_IMPAIR_MANA))
		percent /= 2;

	/* Regenerate mana */
	new_mana = ((long)p->msp) * percent + PY_REGEN_MNBASE;
	p->csp += (s16b)(new_mana >> 16);	/* div 65536 */

	/* check for overflow */
	if ((p->csp < 0) && (old_csp > 0)) {
		p->csp = SHRT_MAX;
	}
	new_mana_frac = (new_mana & 0xFFFF) + p->csp_frac;	/* mod 65536 */
	if (new_mana_frac >= 0x10000L) {
		p->csp_frac = (u16b)(new_mana_frac - 0x10000L);
		p->csp++;
	} else {
		p->csp_frac = (u16b)new_mana_frac;
	}

	/* Must set frac to zero even if equal */
	if (p->csp >= p->msp) {
		p->csp = p->msp;
		p->csp_frac = 0;
	}

	/* Notice changes */
	if (old_csp != p->csp) {
		p->upkeep->redraw |= (PR_MANA);
		equip_learn_flag(p, OF_REGEN);
		equip_learn_flag(p, OF_IMPAIR_MANA);
	}
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
				disturb(p, 0);
				msg("Your light has gone out!");

				/* If it's a torch, now is the time to delete it */
				if (of_has(obj->flags, OF_BURNS_OUT)) {
					bool dummy;
					struct object *burnt = gear_object_for_use(obj, 1, false,
															   &dummy);
					if (burnt->known)
						object_delete(&burnt->known);
					object_delete(&burnt);
				}
			} else if ((obj->timeout < 50) && (!(obj->timeout % 20))) {
				/* The light is getting dim */
				disturb(p, 0);
				msg("Your light is growing faint.");
			}
		}
	}

	/* Calculate torch radius */
	p->upkeep->update |= (PU_TORCH);
}

/**
 * See how much damage the player will take from damaging terrain
 */
int player_check_terrain_damage(struct player *p, int y, int x)
{
	int dam_taken = 0;

	if (square_isfiery(cave, y, x)) {
		int base_dam = 100 + randint1(100);
		int res = p->state.el_info[ELEM_FIRE].res_level;

		/* Fire damage */
		dam_taken = adjust_dam(p, ELEM_FIRE, base_dam, RANDOMISE, res, false);

		/* Feather fall makes one lightfooted. */
		if (player_of_has(p, OF_FEATHER)) {
			dam_taken /= 2;
		}
	}

	return dam_taken;
}

/**
 * Terrain damages the player
 */
void player_take_terrain_damage(struct player *p, int y, int x)
{
	int dam_taken = player_check_terrain_damage(p, y, x);

	if (!dam_taken) {
		return;
	}

	/* Damage the player and inventory */
	take_hit(player, dam_taken, square_feat(cave, y, x)->die_msg);
	if (square_isfiery(cave, y, x)) {
		msg(square_feat(cave, y, x)->hurt_msg);
		inven_damage(player, PROJ_FIRE, dam_taken);
	}
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
 * Revert to normal shape
 */
void player_resume_normal_shape(struct player *p)
{
	p->shape = lookup_player_shape("normal");
	msg("You resume your usual shape.");

	/* Update */
	player->upkeep->update |= (PU_BONUS);
	player->upkeep->redraw |= (PR_TITLE | PR_MISC);
	handle_stuff(player);
}

/**
 * Check if the player is shapechanged
 */
bool player_is_shapechanged(struct player *p)
{
	return streq(p->shape->name, "normal") ? false : true;
}

/**
 * Check if the player is immune from traps
 */
bool player_is_trapsafe(struct player *p)
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
bool player_can_cast(struct player *p, bool show_msg)
{
	if (!p->class->magic.total_spells) {
		if (show_msg) {
			msg("You cannot pray or produce magics.");
		}
		return false;
	}

	if (p->timed[TMD_BLIND] || no_light()) {
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
bool player_can_study(struct player *p, bool show_msg)
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
bool player_can_read(struct player *p, bool show_msg)
{
	if (p->timed[TMD_BLIND]) {
		if (show_msg)
			msg("You can't see anything.");

		return false;
	}

	if (no_light()) {
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
 * Prerequiste function for command. See struct cmd_info in cmd-process.c.
 */
bool player_can_cast_prereq(void)
{
	return player_can_cast(player, true);
}

/**
 * Prerequiste function for command. See struct cmd_info in cmd-process.c.
 */
bool player_can_study_prereq(void)
{
	return player_can_study(player, true);
}

/**
 * Prerequiste function for command. See struct cmd_info in cmd-process.c.
 */
bool player_can_read_prereq(void)
{
	return player_can_read(player, true);
}

/**
 * Prerequiste function for command. See struct cmd_info in cmd-process.c.
 */
bool player_can_fire_prereq(void)
{
	return player_can_fire(player, true);
}

/**
 * Prerequiste function for command. See struct cmd_info in cmd-process.c.
 */
bool player_can_refuel_prereq(void)
{
	return player_can_refuel(player, true);
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
	item_num = scan_items(item_list, item_max, USE_INVEN | USE_FLOOR,
						  obj_can_study);
	for (i = 0; i < item_num; i++) {
		const struct class_book *book = player_object_to_book(p, item_list[i]);
		if (!book) continue;

		/* Extract spells */
		for (j = 0; j < book->num_spells; j++)
			if (spell_okay_to_study(book->spells[j].sidx)) {
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

	if (p->timed[TMD_CONFUSED])
		if ((dir == 5) || (randint0(100) < 75))
			/* Random direction */
			dir = ddd[randint0(8)];

	if (*dp != dir) {
		if (too)
			msg("You are too confused.");
		else
			msg("You are confused.");

		*dp = dir;
		return true;
	}

	return false;
}

/**
 * Return true if the provided count is one of the conditional REST_ flags.
 */
bool player_resting_is_special(s16b count)
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
bool player_is_resting(struct player *p)
{
	return (p->upkeep->resting > 0 ||
			player_resting_is_special(p->upkeep->resting));
}

/**
 * Return the remaining number of resting turns.
 */
s16b player_resting_count(struct player *p)
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
void player_resting_set_count(struct player *p, s16b count)
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
bool player_resting_can_regenerate(struct player *p)
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
	if (player_resting_is_special(p->upkeep->resting)) {
		if (p->upkeep->resting == REST_ALL_POINTS) {
			if ((p->chp == p->mhp) && (p->csp == p->msp))
				/* Stop resting */
				disturb(p, 0);
		} else if (p->upkeep->resting == REST_COMPLETE) {
			if ((p->chp == p->mhp) && (p->csp == p->msp) &&
				!p->timed[TMD_BLIND] && !p->timed[TMD_CONFUSED] &&
				!p->timed[TMD_POISONED] && !p->timed[TMD_AFRAID] &&
				!p->timed[TMD_TERROR] && !p->timed[TMD_STUN] &&
				!p->timed[TMD_CUT] && !p->timed[TMD_SLOW] &&
				!p->timed[TMD_PARALYZED] && !p->timed[TMD_IMAGE] &&
				!p->word_recall && !p->deep_descent)
				/* Stop resting */
				disturb(p, 0);
		} else if (p->upkeep->resting == REST_SOME_POINTS) {
			if ((p->chp == p->mhp) || (p->csp == p->msp))
				/* Stop resting */
				disturb(p, 0);
		}
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
void player_set_resting_repeat_count(struct player *p, s16b count)
{
	player_resting_repeat_count = count;
}

/**
 * Check if the player state has the given OF_ flag.
 */
bool player_of_has(struct player *p, int flag)
{
	assert(p);
	return of_has(p->state.flags, flag);
}

/**
 * Check if the player resists (or better) an element
 */
bool player_resists(struct player *p, int element)
{
	return (p->state.el_info[element].res_level > 0);
}

/**
 * Check if the player resists (or better) an element
 */
bool player_is_immune(struct player *p, int element)
{
	return (p->state.el_info[element].res_level == 3);
}

/*
 * Extract a "direction" which will move one step from the player location
 * towards the given "target" location (or "5" if no motion necessary).
 */
int coords_to_dir(struct player *p, int y, int x)
{
	return (motion_dir(p->py, p->px, y, x));
}

/**
 * Places the player at the given coordinates in the cave.
 */
void player_place(struct chunk *c, struct player *p, int y, int x)
{
	assert(!c->squares[y][x].mon);

	/* Save player location */
	p->py = y;
	p->px = x;

	/* Mark cave grid */
	c->squares[y][x].mon = -1;

	/* Clear stair creation */
	p->upkeep->create_down_stair = false;
	p->upkeep->create_up_stair = false;
}



/*
 * Something has happened to disturb the player.
 *
 * The first arg indicates a major disturbance, which affects search.
 *
 * The second arg is currently unused, but could induce output flush.
 *
 * All disturbance cancels repeated commands, resting, and running.
 * 
 * XXX-AS: Make callers either pass in a command
 * or call cmd_cancel_repeat inside the function calling this
 */
void disturb(struct player *p, int stop_search)
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
	int y, x;

	/* Various conditions mean no searching */
	if (p->timed[TMD_BLIND] || no_light() ||
		p->timed[TMD_CONFUSED] || p->timed[TMD_IMAGE])
		return;

	/* Search the nearby grids, which are always in bounds */
	for (y = (p->py - 1); y <= (p->py + 1); y++) {
		for (x = (p->px - 1); x <= (p->px + 1); x++) {
			struct object *obj;

			/* Secret doors */
			if (square_issecretdoor(cave, y, x)) {
				msg("You have found a secret door.");
				place_closed_door(cave, y, x);
				disturb(p, 0);
			}

			/* Traps on chests */
			for (obj = square_object(cave, y, x); obj; obj = obj->next) {
				if (!obj->known || !is_trapped_chest(obj))
					continue;

				if (obj->known->pval != obj->pval) {
					msg("You have discovered a trap on the chest!");
					obj->known->pval = obj->pval;
					disturb(p, 0);
				}
			}
		}
	}
}
