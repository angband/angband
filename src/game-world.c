/**
 * \file game-world.c
 * \brief Game core management of the game world
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
#include "dungeon.h"
#include "game-world.h"
#include "init.h"
#include "mon-make.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-timed.h"
#include "player-util.h"
#include "tables.h"

/**
 * Say whether it's daytime or not
 */
bool is_daytime(void)
{
	if ((turn % (10L * z_info->day_length)) < ((10L * z_info->day_length) / 2)) 
		return TRUE;

	return FALSE;
} 


/**
 * If player has inscribed the object with "!!", let him know when it's
 * recharged. -LM-
 * Also inform player when first item of a stack has recharged. -HK-
 * Notify all recharges w/o inscription if notify_recharge option set -WP-
 */
static void recharged_notice(const object_type *obj, bool all)
{
	char o_name[120];

	const char *s;

	bool notify = FALSE;

	if (OPT(notify_recharge)) {
		notify = TRUE;
	} else if (obj->note) {
		/* Find a '!' */
		s = strchr(quark_str(obj->note), '!');

		/* Process notification request */
		while (s) {
			/* Find another '!' */
			if (s[1] == '!') {
				notify = TRUE;
				break;
			}

			/* Keep looking for '!'s */
			s = strchr(s + 1, '!');
		}
	}

	if (!notify) return;

	/* Describe (briefly) */
	object_desc(o_name, sizeof(o_name), obj, ODESC_BASE);

	/* Disturb the player */
	disturb(player, 0);

	/* Notify the player */
	if (obj->number > 1) {
		if (all) msg("Your %s have recharged.", o_name);
		else msg("One of your %s has recharged.", o_name);
	} else if (obj->artifact)
		msg("The %s has recharged.", o_name);
	else
		msg("Your %s has recharged.", o_name);
}


/**
 * Recharge activatable objects in the player's equipment
 * and rods in the inventory and on the ground.
 */
static void recharge_objects(void)
{
	int y, x;

	bool discharged_stack;

	struct object *obj;

	/* Recharge carried gear */
	for (obj = player->gear; obj; obj = obj->next) {
		/* Skip non-objects */
		assert(obj->kind);

		/* Recharge equipment */
		if (object_is_equipped(player->body, obj)) {
			/* Recharge activatable objects */
			if (recharge_timeout(obj)) {
				/* Message if an item recharged */
				recharged_notice(obj, TRUE);

				/* Window stuff */
				player->upkeep->redraw |= (PR_EQUIP);
			}
		} else {
			/* Recharge the inventory */
			discharged_stack =
				(number_charging(obj) == obj->number) ? TRUE : FALSE;

			/* Recharge rods, and update if any rods are recharged */
			if (tval_can_have_timeout(obj) && recharge_timeout(obj)) {
				/* Entire stack is recharged */
				if (obj->timeout == 0)
					recharged_notice(obj, TRUE);

				/* Previously exhausted stack has acquired a charge */
				else if (discharged_stack)
					recharged_notice(obj, FALSE);

				/* Combine pack */
				player->upkeep->notice |= (PN_COMBINE);

				/* Redraw stuff */
				player->upkeep->redraw |= (PR_INVEN);
			}
		}
	}

	/* Recharge the ground */
	for (y = 1; y < cave->height; y++)
		for (x = 1; x < cave->width; x++)
			for (obj = square_object(cave, y, x); obj; obj = obj->next)
				/* Recharge rods on the ground */
				if (tval_can_have_timeout(obj))
					recharge_timeout(obj);
}


/**
 * Play an ambient sound dependent on dungeon level, and day or night in town
 */
void play_ambient_sound(void)
{
	if (player->depth == 0) {
		if (is_daytime())
			sound(MSG_AMBIENT_DAY);
		else 
			sound(MSG_AMBIENT_NITE);
	} else if (player->depth <= 20) {
		sound(MSG_AMBIENT_DNG1);
	} else if (player->depth <= 40) {
		sound(MSG_AMBIENT_DNG2);
	} else if (player->depth <= 60) {
		sound(MSG_AMBIENT_DNG3);
	} else if (player->depth <= 80) {
		sound(MSG_AMBIENT_DNG4);
	} else {
		sound(MSG_AMBIENT_DNG5);
	}
}

/**
 * Helper for process_world -- decrement player->timed[] fields.
 */
static void decrease_timeouts(void)
{
	int adjust = (adj_con_fix[player->state.stat_ind[STAT_CON]] + 1);
	int i;

	/* Most effects decrement by 1 */
	for (i = 0; i < TMD_MAX; i++) {
		int decr = 1;
		if (!player->timed[i])
			continue;

		/* Special cases */
		switch (i) {
			case TMD_CUT:
			{
				/* Hack -- check for truly "mortal" wound */
				decr = (player->timed[i] > 1000) ? 0 : adjust;
				break;
			}

			case TMD_POISONED:
			case TMD_STUN:
			{
				decr = adjust;
				break;
			}
		}
		/* Decrement the effect */
		player_dec_timed(player, i, decr, FALSE);
	}

	return;
}


/**
 * Handle things that need updating once every 10 game turns
 */
void process_world(struct chunk *c)
{
	int i;

	/*** Check the Time ***/

	/* Play an ambient sound at regular intervals. */
	if (!(turn % ((10L * z_info->day_length) / 4)))
		play_ambient_sound();

	/*** Handle stores and sunshine ***/

	if (!player->depth) {
		/* Daybreak/Nighfall in town */
		if (!(turn % ((10L * z_info->day_length) / 2))) {
			bool dawn;

			/* Check for dawn */
			dawn = (!(turn % (10L * z_info->day_length)));

			/* Day breaks */
			if (dawn)
				msg("The sun has risen.");

			/* Night falls */
			else
				msg("The sun has fallen.");

			/* Illuminate */
			cave_illuminate(c, dawn);
		}
	} else {
		/* Update the stores once a day (while in the dungeon).
		   The changes are not actually made until return to town,
		   to avoid giving details away in the knowledge menu. */
		if (!(turn % (10L * z_info->store_turns))) daycount++;
	}


	/* Check for creature generation */
	if (one_in_(z_info->alloc_monster_chance))
		(void)pick_and_place_distant_monster(cave, loc(player->px, player->py),
											 z_info->max_sight + 5, TRUE,
											 player->depth);

	/*** Damage over Time ***/

	/* Take damage from poison */
	if (player->timed[TMD_POISONED])
		take_hit(player, 1, "poison");

	/* Take damage from cuts */
	if (player->timed[TMD_CUT]) {
		/* Mortal wound or Deep Gash */
		if (player->timed[TMD_CUT] > 200)
			i = 3;

		/* Severe cut */
		else if (player->timed[TMD_CUT] > 100)
			i = 2;

		/* Other cuts */
		else
			i = 1;

		/* Take damage */
		take_hit(player, i, "a fatal wound");
	}


	/*** Check the Food, and Regenerate ***/

	/* Digest normally */
	if (!(turn % 100)) {
		/* Basic digestion rate based on speed */
		i = extract_energy[player->state.speed] * 2;

		/* Regeneration takes more food */
		if (player_of_has(player, OF_REGEN)) i += 30;

		/* Slow digestion takes less food */
		if (player_of_has(player, OF_SLOW_DIGEST)) i /= 5;

		/* Minimal digestion */
		if (i < 1) i = 1;

		/* Digest some food */
		player_set_food(player, player->food - i);
	}

	/* Getting Faint */
	if (player->food < PY_FOOD_FAINT) {
		/* Faint occasionally */
		if (!player->timed[TMD_PARALYZED] && one_in_(10)) {
			/* Message */
			msg("You faint from the lack of food.");
			disturb(player, 1);

			/* Faint (bypass free action) */
			(void)player_inc_timed(player, TMD_PARALYZED, 1 + randint0(5),
								   TRUE, FALSE);
		}
	}

	/* Starve to death (slowly) */
	if (player->food < PY_FOOD_STARVE) {
		/* Calculate damage */
		i = (PY_FOOD_STARVE - player->food) / 10;

		/* Take damage */
		take_hit(player, i, "starvation");
	}

	/* Regenerate Hit Points if needed */
	if (player->chp < player->mhp)
		player_regen_hp();

	/* Regenerate mana if needed */
	if (player->csp < player->msp)
		player_regen_mana();

	/* Timeout various things */
	decrease_timeouts();

	/* Process light */
	player_update_light();


	/*** Process Inventory ***/

	/* Handle experience draining */
	if (player_of_has(player, OF_DRAIN_EXP)) {
		if ((player->exp > 0) && one_in_(10)) {
			s32b d = damroll(10, 6) +
				(player->exp / 100) * z_info->life_drain_percent;
			player_exp_lose(player, d / 10, FALSE);
		}

		equip_notice_flag(player, OF_DRAIN_EXP);
	}

	/* Recharge activatable objects and rods */
	recharge_objects();

	/* Feel the inventory */
	sense_inventory();


	/*** Involuntary Movement ***/

	/* Random teleportation */
	if (player_of_has(player, OF_TELEPORT) && one_in_(50)) {
		const char *forty = "40";
		equip_notice_flag(player, OF_TELEPORT);
		effect_simple(EF_TELEPORT, forty, 0, 1, 0, NULL);
		disturb(player, 0);
	}

	/* Delayed Word-of-Recall */
	if (player->word_recall) {
		/* Count down towards recall */
		player->word_recall--;

		/* Activate the recall */
		if (!player->word_recall) {
			/* Disturbing! */
			disturb(player, 0);

			/* Determine the level */
			if (player->depth) {
				msgt(MSG_TPLEVEL, "You feel yourself yanked upwards!");
				dungeon_change_level(0);
			} else {
				msgt(MSG_TPLEVEL, "You feel yourself yanked downwards!");
                
                /* Force descent to a lower level if allowed */
                if (OPT(birth_force_descend) &&
					player->max_depth < z_info->max_depth - 1 &&
					!is_quest(player->max_depth)) {
                    player->max_depth = player->max_depth + 1;
                }

				/* New depth - back to max depth or 1, whichever is deeper */
				dungeon_change_level(player->max_depth < 1 ? 1: player->max_depth);
			}
		}
	}

	/* Delayed Deep Descent */
	if (player->deep_descent) {
		/* Count down towards recall */
		player->deep_descent--;

		/* Activate the recall */
		if (player->deep_descent == 0) {
			int target_depth = player->max_depth;

			/* Calculate target depth */
			for (i = 5; i > 0; i--) {
				if (is_quest(target_depth)) break;
				if (target_depth >= z_info->max_depth - 1) break;
				
				target_depth++;
			}

			disturb(player, 0);

			/* Determine the level */
			if (target_depth > player->depth) {
				msgt(MSG_TPLEVEL, "The floor opens beneath you!");
				dungeon_change_level(target_depth);
			} else {
				/* Otherwise do something disastrous */
				msgt(MSG_TPLEVEL, "You are thrown back in an explosion!");
				effect_simple(EF_DESTRUCTION, "0", 0, 5, 0, NULL);
			}		
		}
	}
}

