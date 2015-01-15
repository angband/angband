/*
 * File: dungeon.c
 * Purpose: The game core bits, shared across platforms.
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
#include "cave.h"
#include "cmds.h"
#include "dungeon.h"
#include "game-event.h"
#include "game-input.h"
#include "generate.h"
#include "grafmode.h"
#include "init.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-move.h"
#include "mon-spell.h"
#include "mon-util.h"
#include "monster.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-pile.h"
#include "obj-randart.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "object.h"
#include "player-birth.h"
#include "player-history.h"
#include "player-path.h"
#include "player-timed.h"
#include "player-util.h"
#include "savefile.h"
#include "score.h"
#include "signals.h"
#include "store.h"
#include "tables.h"
#include "target.h"
#include "ui-birth.h"
#include "ui-death.h"
#include "ui-input.h"
#include "ui-map.h"
#include "ui-mon-list.h"
#include "ui-prefs.h"
#include "ui-score.h"
#include "ui.h"

/* The minimum amount of energy a player has at the start of a new level */
#define INITIAL_DUNGEON_ENERGY 100

u16b daycount = 0;
u32b seed_randart;		/* Hack -- consistent random artifacts */
u32b seed_flavor;		/* Hack -- consistent object colors */
s32b turn;				/* Current game turn */
bool character_generated;	/* The character exists */
bool character_dungeon;		/* The character has a dungeon */
bool character_saved;		/* The character was just saved to a savefile */
s16b character_xtra;		/* Depth of the game in startup mode */
bool arg_wizard;			/* Command arg -- Request wizard mode */

/*
 * Say whether it's daytime or not
 */
bool is_daytime(void)
{
	if ((turn % (10L * z_info->day_length)) < ((10L * z_info->day_length) / 2)) 
		return TRUE;

	return FALSE;
} 


/*
 * Change dungeon level - e.g. by going up stairs or with WoR.
 */
void dungeon_change_level(int dlev)
{
	/* New depth */
	player->depth = dlev;

	/* If we're returning to town, update the store contents
	   according to how long we've been away */
	if (!dlev && daycount)
		store_update();

	/* Leaving, make new level */
	player->upkeep->generate_level = TRUE;

	/* Save the game when we arrive on the new level. */
	player->upkeep->autosave = TRUE;
}


/*
 * If player has inscribed the object with "!!", let him know when it's
 * recharged. -LM-
 * Also inform player when first item of a stack has recharged. -HK-
 * Notify all recharges w/o inscription if notify_recharge option set -WP-
 */
static void recharged_notice(const object_type *o_ptr, bool all)
{
	char o_name[120];

	const char *s;

	bool notify = FALSE;

	if (OPT(notify_recharge))
	{
		notify = TRUE;
	}
	else if (o_ptr->note)
	{
		/* Find a '!' */
		s = strchr(quark_str(o_ptr->note), '!');

		/* Process notification request */
		while (s)
		{
			/* Find another '!' */
			if (s[1] == '!')
			{
				notify = TRUE;
				break;
			}

			/* Keep looking for '!'s */
			s = strchr(s + 1, '!');
		}
	}

	if (!notify) return;


	/* Describe (briefly) */
	object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);

	/* Disturb the player */
	disturb(player, 0);

	/* Notify the player */
	if (o_ptr->number > 1)
	{
		if (all) msg("Your %s have recharged.", o_name);
		else msg("One of your %s has recharged.", o_name);
	}

	/* Artifacts */
	else if (o_ptr->artifact)
	{
		msg("The %s has recharged.", o_name);
	}

	/* Single, non-artifact items */
	else msg("Your %s has recharged.", o_name);
}


/*
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
static void play_ambient_sound(void)
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
static void process_world(struct chunk *c)
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

/*
 * Place cursor on a monster or the player.
 */
static void place_cursor(void) {
	if (OPT(show_target) && target_sighted()) {
		int col, row;
		target_get(&col, &row);
		move_cursor_relative(row, col);
	}
}


/*
 * Process the player
 *
 * Notice the annoying code to handle "pack overflow", which
 * must come first just in case somebody manages to corrupt
 * the savefiles by clever use of menu commands or something.
 *
 * Notice the annoying code to handle "monster memory" changes,
 * which allows us to avoid having to update the window flags
 * every time we change any internal monster memory field, and
 * also reduces the number of times that the recall window must
 * be redrawn.
 *
 * Note that the code to check for user abort during repeated commands
 * and running and resting can be disabled entirely with an option, and
 * even if not disabled, it will only check during every 128th game turn
 * while resting, for efficiency.
 */
static void process_player(void)
{
	int i;

	/*** Check for interrupts ***/

	player_resting_complete_special(player);

	/* Check for "player abort" */
	if (player->upkeep->running ||
	    cmd_get_nrepeats() > 0 ||
	    (player_is_resting(player) && !(turn & 0x7F)))
	{
		ui_event e;

		/* Do not wait */
		inkey_scan = SCAN_INSTANT;

		/* Check for a key */
		e = inkey_ex();
		if (e.type != EVT_NONE) {
			/* Flush and disturb */
			event_signal(EVENT_INPUT_FLUSH);
			disturb(player, 0);
			msg("Cancelled.");
		}
	}


	/*** Handle actual user input ***/

	/* Mega hack -redraw big graphics - sorry NRM */
	if ((tile_width > 1) || (tile_height > 1))
		player->upkeep->redraw |= (PR_MAP);

	/* Repeat until energy is reduced */
	do
	{
		/* Notice stuff (if needed) */
		if (player->upkeep->notice) notice_stuff(player->upkeep);

		/* Update stuff (if needed) */
		if (player->upkeep->update) update_stuff(player->upkeep);

		/* Redraw stuff (if needed) */
		if (player->upkeep->redraw) redraw_stuff(player->upkeep);

		/* Place cursor on player/target */
		place_cursor();

		/* Refresh (optional) */
		Term_fresh();

		/* Hack -- Pack Overflow */
		pack_overflow();

		/* Assume free turn */
		player->upkeep->energy_use = 0;

		/* Dwarves detect treasure */
		if (player_has(PF_SEE_ORE)) {
			/* Only if they are in good shape */
			if (!player->timed[TMD_IMAGE] &&
					!player->timed[TMD_CONFUSED] &&
					!player->timed[TMD_AMNESIA] &&
					!player->timed[TMD_STUN] &&
					!player->timed[TMD_PARALYZED] &&
					!player->timed[TMD_TERROR] &&
					!player->timed[TMD_AFRAID])
				effect_simple(EF_DETECT_GOLD, "3d3", 1, 0, 0, NULL);
		}

		/* Paralyzed or Knocked Out player gets no turn */
		if ((player->timed[TMD_PARALYZED]) || (player->timed[TMD_STUN] >= 100))
			cmdq_push(CMD_SLEEP);

		/* Repeated command */
		if (cmd_get_nrepeats() > 0) {
			/* Hack -- Assume messages were seen */
			msg_flag = FALSE;

			/* Clear the top line */
			prt("", 0, 0);

			/* Process the command */
			process_command(CMD_GAME, TRUE);
		} else { /* Normal command */
			/* Check monster recall */
			if (player->upkeep->monster_race)
				player->upkeep->redraw |= (PR_MONSTER);

			/* Place cursor on player/target */
			place_cursor();

			/* Get and process a command */
			if (cmdq_is_empty())
				process_command(CMD_GAME, FALSE);
			else
				process_command(CMD_GAME, TRUE);

			if (!player->upkeep->playing)
				break;

			/* Mega hack - redraw if big graphics - sorry NRM */
			if ((tile_width > 1) || (tile_height > 1)) 
				player->upkeep->redraw |= (PR_MAP);
		}


		/*** Clean up ***/

		/* Significant */
		if (player->upkeep->energy_use) {
			/* Use some energy */
			player->energy -= player->upkeep->energy_use;

			/* Increment the total energy counter */
			player->total_energy += player->upkeep->energy_use;

			/* Hack -- constant hallucination */
			if (player->timed[TMD_IMAGE])
				player->upkeep->redraw |= (PR_MAP);

			/* Shimmer multi-hued monsters */
			for (i = 1; i < cave_monster_max(cave); i++) {
				struct monster *mon = cave_monster(cave, i);
				if (!mon->race)
					continue;
				if (!rf_has(mon->race->flags, RF_ATTR_MULTI))
					continue;
				square_light_spot(cave, mon->fy, mon->fx);
			}

			/* Clear NICE flag, and show marked monsters */
			for (i = 1; i < cave_monster_max(cave); i++) {
				struct monster *mon = cave_monster(cave, i);
				mflag_off(mon->mflag, MFLAG_NICE);
				if (mflag_has(mon->mflag, MFLAG_MARK)) {
					if (!mflag_has(mon->mflag, MFLAG_SHOW)) {
						mflag_off(mon->mflag, MFLAG_MARK);
						update_mon(mon, cave, FALSE);
					}
				}
			}
		}

		/* Clear SHOW flag */
		for (i = 1; i < cave_monster_max(cave); i++) {
			struct monster *mon = cave_monster(cave, i);
			mflag_off(mon->mflag, MFLAG_SHOW);
		}

		/* HACK: This will redraw the itemlist too frequently, but I'm don't
		   know all the individual places it should go. */
		player->upkeep->redraw |= PR_ITEMLIST;
		/* Hack - update needed first because inventory may have changed */
		update_stuff(player->upkeep);
		redraw_stuff(player->upkeep);
	}

	while (!player->upkeep->energy_use && !player->is_dead && !player->upkeep->generate_level);

	/* Notice stuff (if needed) */
	if (player->upkeep->notice) notice_stuff(player->upkeep);
}

static byte flicker = 0;
static byte color_flicker[MAX_COLORS][3] = 
{
	{COLOUR_DARK, COLOUR_L_DARK, COLOUR_L_RED},
	{COLOUR_WHITE, COLOUR_L_WHITE, COLOUR_L_BLUE},
	{COLOUR_SLATE, COLOUR_WHITE, COLOUR_L_DARK},
	{COLOUR_ORANGE, COLOUR_YELLOW, COLOUR_L_RED},
	{COLOUR_RED, COLOUR_L_RED, COLOUR_L_PINK},
	{COLOUR_GREEN, COLOUR_L_GREEN, COLOUR_L_TEAL},
	{COLOUR_BLUE, COLOUR_L_BLUE, COLOUR_SLATE},
	{COLOUR_UMBER, COLOUR_L_UMBER, COLOUR_MUSTARD},
	{COLOUR_L_DARK, COLOUR_SLATE, COLOUR_L_VIOLET},
	{COLOUR_WHITE, COLOUR_SLATE, COLOUR_L_WHITE},
	{COLOUR_L_PURPLE, COLOUR_PURPLE, COLOUR_L_VIOLET},
	{COLOUR_YELLOW, COLOUR_L_YELLOW, COLOUR_MUSTARD},
	{COLOUR_L_RED, COLOUR_RED, COLOUR_L_PINK},
	{COLOUR_L_GREEN, COLOUR_L_TEAL, COLOUR_GREEN},
	{COLOUR_L_BLUE, COLOUR_DEEP_L_BLUE, COLOUR_BLUE_SLATE},
	{COLOUR_L_UMBER, COLOUR_UMBER, COLOUR_MUD},
	{COLOUR_PURPLE, COLOUR_VIOLET, COLOUR_MAGENTA},
	{COLOUR_VIOLET, COLOUR_L_VIOLET, COLOUR_MAGENTA},
	{COLOUR_TEAL, COLOUR_L_TEAL, COLOUR_L_GREEN},
	{COLOUR_MUD, COLOUR_YELLOW, COLOUR_UMBER},
	{COLOUR_L_YELLOW, COLOUR_WHITE, COLOUR_L_UMBER},
	{COLOUR_MAGENTA, COLOUR_L_PINK, COLOUR_L_RED},
	{COLOUR_L_TEAL, COLOUR_L_WHITE, COLOUR_TEAL},
	{COLOUR_L_VIOLET, COLOUR_L_PURPLE, COLOUR_VIOLET},
	{COLOUR_L_PINK, COLOUR_L_RED, COLOUR_L_WHITE},
	{COLOUR_MUSTARD, COLOUR_YELLOW, COLOUR_UMBER},
	{COLOUR_BLUE_SLATE, COLOUR_BLUE, COLOUR_SLATE},
	{COLOUR_DEEP_L_BLUE, COLOUR_L_BLUE, COLOUR_BLUE},
};

static byte get_flicker(byte a)
{
	switch(flicker % 3)
	{
		case 1: return color_flicker[a][1];
		case 2: return color_flicker[a][2];
	}
	return a;
}

/**
 * This animates monsters and/or items as necessary.
 */
void do_animation(void)
{
	int i;

	for (i = 1; i < cave_monster_max(cave); i++)
	{
		byte attr;
		monster_type *m_ptr = cave_monster(cave, i);

		if (!m_ptr || !m_ptr->race || !mflag_has(m_ptr->mflag, MFLAG_VISIBLE))
			continue;
		else if (rf_has(m_ptr->race->flags, RF_ATTR_MULTI))
			attr = randint1(BASIC_COLORS - 1);
		else if (rf_has(m_ptr->race->flags, RF_ATTR_FLICKER))
			attr = get_flicker(monster_x_attr[m_ptr->race->ridx]);
		else
			continue;

		m_ptr->attr = attr;
		player->upkeep->redraw |= (PR_MAP | PR_MONLIST);
	}

	flicker++;
}


static void refresh(void)
{
	/* Notice stuff */
	if (player->upkeep->notice)
		notice_stuff(player->upkeep);

	/* Update stuff */
	if (player->upkeep->update)
		update_stuff(player->upkeep);

	/* Redraw stuff */
	if (player->upkeep->redraw)
		redraw_stuff(player->upkeep);

	/* Place cursor on player/target */
	place_cursor();
}

static void on_new_level(void)
{
	/* Play ambient sound on change of level. */
	play_ambient_sound();

	/* Hack -- enforce illegal panel */
	Term->offset_y = z_info->dungeon_hgt;
	Term->offset_x = z_info->dungeon_wid;


	/* Cancel the target */
	target_set_monster(0);

	/* Cancel the health bar */
	health_track(player->upkeep, NULL);

	/* Disturb */
	disturb(player, 1);

	/*
	 * Because changing levels doesn't take a turn and PR_MONLIST might not be
	 * set for a few game turns, manually force an update on level change.
	 */
	monster_list_force_subwindow_update();

	/* Track maximum player level */
	if (player->max_lev < player->lev)
		player->max_lev = player->lev;


	/* Track maximum dungeon level */
	if (player->max_depth < player->depth)
		player->max_depth = player->depth;

	/* If autosave is pending, do it now. */
	if (player->upkeep->autosave) {
		save_game();
		player->upkeep->autosave = FALSE;
	}

	/* Choose panel */
	verify_panel();

	/* Flush messages */
	event_signal(EVENT_MESSAGE_FLUSH);

	/* Hack -- Increase "xtra" depth */
	character_xtra++;

	/* Clear */
	Term_clear();

	/* Update stuff */
	player->upkeep->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

	/* Calculate torch radius */
	player->upkeep->update |= (PU_TORCH);

	/* Update stuff */
	update_stuff(player->upkeep);

	/* Fully update the visuals (and monster distances) */
	player->upkeep->update |= (PU_FORGET_VIEW | PU_UPDATE_VIEW | PU_DISTANCE);

	/* Fully update the flow */
	player->upkeep->update |= (PU_FORGET_FLOW | PU_UPDATE_FLOW);

	/* Redraw dungeon */
	player->upkeep->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP);

	/* Redraw "statusy" things */
	player->upkeep->redraw |= (PR_INVEN | PR_EQUIP | PR_MONSTER | PR_MONLIST | PR_ITEMLIST);

	/* Update stuff */
	update_stuff(player->upkeep);

	/* Redraw stuff */
	redraw_stuff(player->upkeep);

	/* Hack -- Decrease "xtra" depth */
	character_xtra--;

	/* Update stuff */
	player->upkeep->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS | PU_INVEN);

	/* Combine the pack */
	player->upkeep->notice |= (PN_COMBINE);

	/* Notice stuff */
	notice_stuff(player->upkeep);

	/* Update stuff */
	update_stuff(player->upkeep);

	/* Redraw stuff */
	redraw_stuff(player->upkeep);

	/* Refresh */
	Term_fresh();

	/* Announce (or repeat) the feeling */
	if (player->depth) display_feeling(FALSE);

	/* Give player minimum energy to start a new level, but do not reduce
	 * higher value from savefile for level in progress */
	if (player->energy < INITIAL_DUNGEON_ENERGY)
		player->energy = INITIAL_DUNGEON_ENERGY;
}

static void on_leave_level(void) {
	/* Notice stuff */
	if (player->upkeep->notice) notice_stuff(player->upkeep);
	if (player->upkeep->update) update_stuff(player->upkeep);
	if (player->upkeep->redraw) redraw_stuff(player->upkeep);

	forget_view(cave);

	/* XXX XXX XXX */
	event_signal(EVENT_MESSAGE_FLUSH);
}


/*
 * Actually play a game.
 *
 * This function is called from a variety of entry points, since both
 * the standard "main.c" file, as well as several platform-specific
 * "main-xxx.c" files, call this function to start a new game with a
 * new savefile, start a new game with an existing savefile, or resume
 * a saved game with an existing savefile.
 *
 * If the "new_game" parameter is true, and the savefile contains a
 * living character, then that character will be killed, so that the
 * player may start a new game with that savefile.  This is only used
 * by the "-n" option in "main.c".
 *
 * If the savefile does not exist, cannot be loaded, or contains a dead
 * character, then a new game will be started.
 */
void play_game(bool new_game)
{
	bool new_level = TRUE;

	/*** Try to load the savefile ***/

	player->is_dead = TRUE;

	/* Try loading */
	if (file_exists(savefile) && !savefile_load(savefile, arg_wizard))
		quit("Broken savefile");

	/* No living character loaded */
	if (player->is_dead || new_game) {
		character_generated = FALSE;
		textui_do_birth();
	}

	/* Tell the UI we've started. */
	event_signal(EVENT_LEAVE_INIT);
	event_signal(EVENT_ENTER_GAME);

	/* Save not required yet. */
	player->upkeep->autosave = FALSE;

	/* Process */
	while (!player->is_dead && player->upkeep->playing) {

		/* Make a new level if requested */
		if (player->upkeep->generate_level) {
			if (character_dungeon)
				on_leave_level();

			cave_generate(&cave, player);

			new_level = TRUE;
			player->upkeep->generate_level = FALSE;
		}

		if (new_level) {
			on_new_level();
			new_level = FALSE;
		}

		/* Can the player move? */
		while (player->energy >= 100) {
			/* Do any necessary animations */
			do_animation();

			/* Process monster with even more energy first */
			process_monsters(cave, player->energy + 1);
			if (player->is_dead || !player->upkeep->playing ||
				player->upkeep->generate_level)
				break;

			/* Process the player */
			process_player();
			if (player->is_dead || !player->upkeep->playing ||
				player->upkeep->generate_level)
				break;
		}

		/* Refresh */
		refresh();
		if (player->is_dead || !player->upkeep->playing ||
			player->upkeep->generate_level)
			continue;

		/* Process the rest of the monsters */
		process_monsters(cave, 0);

		/* Mark all monsters as processed this turn */
		reset_monsters();

		/* Refresh */
		refresh();
		if (player->is_dead || !player->upkeep->playing ||
			player->upkeep->generate_level)
			continue;

		/* Process the world every ten turns */
		if (!(turn % 10)) {
			/* Compact the monster list if we're approaching the limit */
			if (cave_monster_count(cave) + 32 > z_info->level_monster_max)
				compact_monsters(64);

			/* Too many holes in the monster list - compress */
			if (cave_monster_count(cave) + 32 < cave_monster_max(cave))
				compact_monsters(0);			

			process_world(cave);

			/* Refresh */
			refresh();
			if (player->is_dead || !player->upkeep->playing ||
				player->upkeep->generate_level)
				continue;
		}

		/*** Apply energy ***/

		/* Give the player some energy */
		player->energy += extract_energy[player->state.speed];

		/* Count game turns */
		turn++;
	}

	/* Tell the UI we're done with the game state */
	event_signal(EVENT_LEAVE_GAME);

	/* Close stuff */
	close_game();
}

/*
 * Save the game
 */
void save_game(void)
{
	char name[80];
	char path[1024];

	/* Disturb the player */
	disturb(player, 1);

	/* Clear messages */
	event_signal(EVENT_MESSAGE_FLUSH);

	/* Handle stuff */
	handle_stuff(player->upkeep);

	/* Message */
	prt("Saving game...", 0, 0);

	/* Refresh */
	Term_fresh();

	/* The player is not dead */
	my_strcpy(player->died_from, "(saved)", sizeof(player->died_from));

	/* Forbid suspend */
	signals_ignore_tstp();

	/* Save the player */
	if (savefile_save(savefile))
		prt("Saving game... done.", 0, 0);
	else
		prt("Saving game... failed!", 0, 0);

	/* Refresh */
	Term_fresh();

	/* Save the window prefs */
	strnfmt(name, sizeof(name), "%s.prf", player_safe_name(player, TRUE));
	path_build(path, sizeof(path), ANGBAND_DIR_USER, name);
	if (!prefs_save(path, option_dump, "Dump window settings"))
		prt("Failed to save subwindow preferences", 0, 0);

	/* Allow suspend again */
	signals_handle_tstp();

	/* Refresh */
	Term_fresh();

	/* Note that the player is not dead */
	my_strcpy(player->died_from, "(alive and well)", sizeof(player->died_from));
}



/**
 * Win or not, know inventory, home items and history upon death, enter score
 */
static void death_knowledge(void)
{
	struct store *home = &stores[STORE_HOME];
	object_type *obj;
	time_t death_time = (time_t)0;

	/* Retire in the town in a good state */
	if (player->total_winner) {
		player->depth = 0;
		my_strcpy(player->died_from, "Ripe Old Age", sizeof(player->died_from));
		player->exp = player->max_exp;
		player->lev = player->max_lev;
		player->au += 10000000L;
	}

	for (obj = player->gear; obj; obj = obj->next) {
		object_flavor_aware(obj);
		object_notice_everything(obj);
	}

	for (obj = home->stock; obj; obj = obj->next) {
		object_flavor_aware(obj);
		object_notice_everything(obj);
	}

	history_unmask_unknown();

	/* Get time of death */
	(void)time(&death_time);
	enter_score(&death_time);

	/* Hack -- Recalculate bonuses */
	player->upkeep->update |= (PU_BONUS);
	handle_stuff(player->upkeep);
}



/*
 * Close up the current game (player may or may not be dead)
 *
 * Note that the savefile is not saved until the tombstone is
 * actually displayed and the player has a chance to examine
 * the inventory and such.  This allows cheating if the game
 * is equipped with a "quit without save" method.  XXX XXX XXX
 */
void close_game(void)
{
	/* Handle stuff */
	handle_stuff(player->upkeep);

	/* Flush the messages */
	event_signal(EVENT_MESSAGE_FLUSH);

	/* Flush the input */
	event_signal(EVENT_INPUT_FLUSH);


	/* No suspending now */
	signals_ignore_tstp();


	/* Hack -- Increase "icky" depth */
	screen_save_depth++;

	/* Save monster memory to user directory */
	if (!lore_save("lore.txt")) {
		msg("lore save failed!");
		event_signal(EVENT_MESSAGE_FLUSH);
	}

	/* Handle death */
	if (player->is_dead)
	{
		death_knowledge();
		death_screen();

		/* Save dead player */
		if (!savefile_save(savefile))
		{
			msg("death save failed!");
			event_signal(EVENT_MESSAGE_FLUSH);
		}
	}

	/* Still alive */
	else
	{
		/* Save the game */
		save_game();

		if (Term->mapped_flag)
		{
			struct keypress ch;

			prt("Press Return (or Escape).", 0, 40);
			ch = inkey();
			if (ch.code != ESCAPE)
				predict_score();
		}
	}


	/* Hack -- Decrease "icky" depth */
	screen_save_depth--;


	/* Allow suspending now */
	signals_handle_tstp();
}
