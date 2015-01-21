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
#include "game-world.h"
#include "generate.h"
#include "grafmode.h"
#include "init.h"
#include "mon-lore.h"
#include "mon-make.h"
#include "mon-move.h"
#include "mon-spell.h"
#include "mon-util.h"
#include "monster.h"
#include "obj-gear.h"
#include "obj-identify.h"
#include "obj-pile.h"
#include "obj-randart.h"
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
bool arg_wizard;			/* Command arg -- Request wizard mode */

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


/**
 * Prepare for a player command to happen
 *
 * Notice the annoying code to handle "pack overflow", which
 * must come first just in case somebody manages to corrupt
 * the savefiles by clever use of menu commands or something. (Can go? NRM)
 *
 * Notice the annoying code to handle "monster memory" changes,
 * which allows us to avoid having to update the window flags
 * every time we change any internal monster memory field, and
 * also reduces the number of times that the recall window must
 * be redrawn.
 */
static void process_player_pre_command(void)
{
	/* Refresh */
	notice_stuff(player->upkeep);
	handle_stuff(player->upkeep);
	event_signal(EVENT_REFRESH);

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

	/* Prepare for the next command */
	if (cmd_get_nrepeats() > 0) {
		/* Hack -- Assume messages were seen */
		msg_flag = FALSE;

		/* Clear the top line */
		prt("", 0, 0);
	} else {
		/* Check monster recall */
		if (player->upkeep->monster_race)
			player->upkeep->redraw |= (PR_MONSTER);

		/* Place cursor on player/target */
		event_signal(EVENT_REFRESH);
	}
}

/**
 * Housekeeping after the processing of a player command
 */
static void process_player_post_command(void)
{
	int i;

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

	/* Hack - update needed first because inventory may have changed */
	update_stuff(player->upkeep);
	redraw_stuff(player->upkeep);
}


/**
 * Process player commands from the command queue, finishing when there is a
 * command using energy (any regular game command), or we run out of commands
 * and need another from the user, or the character changes level or dies, or
 * the game is stopped.
 */
static void process_player(void)
{
	/* Check for interrupts */
	player_resting_complete_special(player);
	event_signal(EVENT_CHECK_INTERRUPT);

	/* Repeat until energy is reduced */
	do {
		process_player_pre_command();

		/* Get a command from the queue if there is one */
		if (!cmdq_pop(CMD_GAME))
			break;

		if (!player->upkeep->playing)
			break;

		process_player_post_command();
	} while (!player->upkeep->energy_use &&
			 !player->is_dead &&
			 !player->upkeep->generate_level);

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


/**
 * Housekeeping on arriving on a new level
 */
void on_new_level(void)
{
	/* Play ambient sound on change of level. */
	play_ambient_sound();

	/* Cancel the target */
	target_set_monster(0);

	/* Cancel the health bar */
	health_track(player->upkeep, NULL);

	/* Disturb */
	disturb(player, 1);

	/* Track maximum player level */
	if (player->max_lev < player->lev)
		player->max_lev = player->lev;

	/* Track maximum dungeon level */
	if (player->max_depth < player->depth)
		player->max_depth = player->depth;

	/* Flush messages */
	event_signal(EVENT_MESSAGE_FLUSH);

	/* Update display */
	event_signal(EVENT_NEW_LEVEL_DISPLAY);

	/* Update player */
	player->upkeep->update |=
		(PU_BONUS | PU_HP | PU_MANA | PU_SPELLS | PU_INVEN);
	player->upkeep->notice |= (PN_COMBINE);
	notice_stuff(player->upkeep);
	update_stuff(player->upkeep);
	redraw_stuff(player->upkeep);

	/* Refresh */
	event_signal(EVENT_REFRESH);

	/* Announce (or repeat) the feeling */
	if (player->depth)
		display_feeling(FALSE);

	/* Give player minimum energy to start a new level, but do not reduce
	 * higher value from savefile for level in progress */
	if (player->energy < INITIAL_DUNGEON_ENERGY)
		player->energy = INITIAL_DUNGEON_ENERGY;
}

/**
 * Housekeeping on leaving a level
 */
static void on_leave_level(void) {
	/* Any pending processing */
	notice_stuff(player->upkeep);
	update_stuff(player->upkeep);
	redraw_stuff(player->upkeep);

	/* Forget the view */
	forget_view(cave);

	/* Flush messages */
	event_signal(EVENT_MESSAGE_FLUSH);
}

/**
 * Start actually playing a game, either by loading a savefile or creating
 * a new character
 */
void start_game(bool new_game)
{
	/* Player will be resuscitated if living in the savefile */
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

	/* Do new level stuff if we have one */
	if (character_dungeon)
		on_new_level();
}

/**
 * The character takes their turn, preceded by any monsters with more energy.
 *
 * Note that the player's turn will run for as long as there are commands
 * in the command queue or entered by the player which don't use energy
 * - that is, interface-only commands
 */
void player_turn(void)
{
	/* Do any necessary animations */
	do_animation();

	/* Process monster with even more energy first */
	process_monsters(cave, player->energy + 1);
	if (player->is_dead || !player->upkeep->playing ||
		player->upkeep->generate_level)
		return;

	/* Process the player, asking for a command if the queue is empty and
	 * we haven't used any energy yet */
	while (player->upkeep->playing) {
		process_player();
		if (player->upkeep->energy_use)
			break;
		else {
			cmd_get_hook(CMD_GAME);
			process_player_post_command();
		}
	}
}

/**
 * The monsters get to take their turns, the player gets energy, and the
 * game turn count is incremented; every ten turns world and player 
 * housekeeping is done.
 */
void game_turn(void)
{
	/* Process the rest of the monsters */
	process_monsters(cave, 0);

	/* Mark all monsters as processed this turn */
	reset_monsters();

	/* Refresh */
	notice_stuff(player->upkeep);
	handle_stuff(player->upkeep);
	event_signal(EVENT_REFRESH);
	if (player->is_dead || !player->upkeep->playing ||
		player->upkeep->generate_level)
		return;

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
		notice_stuff(player->upkeep);
		handle_stuff(player->upkeep);
		event_signal(EVENT_REFRESH);
		if (player->is_dead || !player->upkeep->playing ||
			player->upkeep->generate_level)
			return;
	}

	/*** Apply energy ***/

	/* Give the player some energy */
	player->energy += extract_energy[player->state.speed];

	/* Count game turns */
	turn++;
}

/**
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
	/* Load a savefile or birth a character, or both */
	start_game(new_game);

	/* Process */
	while (!player->is_dead && player->upkeep->playing) {
		/* Make a new level if requested */
		if (player->upkeep->generate_level) {
			if (character_dungeon)
				on_leave_level();

			cave_generate(&cave, player);
			on_new_level();

			player->upkeep->generate_level = FALSE;
		}

		/* If the player has enough energy to move they now do so, after
		 * any monsters with more energy take their turns */
		while (player->energy >= 100) {
			player_turn();

			if (player->is_dead || !player->upkeep->playing ||
				player->upkeep->generate_level)
				break;
		}

		/* Refresh */
		notice_stuff(player->upkeep);
		handle_stuff(player->upkeep);
		event_signal(EVENT_REFRESH);
		if (player->is_dead || !player->upkeep->playing ||
			player->upkeep->generate_level)
			continue;

		/* Process monsters and the world, give the player energy, increment
		 * the turn counter */
		game_turn();
	}

	/* Shut the game down */
	close_game();
}

/**
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



/**
 * Close up the current game (player may or may not be dead)
 *
 * Note that the savefile is not saved until the tombstone is
 * actually displayed and the player has a chance to examine
 * the inventory and such.  This allows cheating if the game
 * is equipped with a "quit without save" method.  XXX XXX XXX
 */
void close_game(void)
{
	/* Tell the UI we're done with the game state */
	event_signal(EVENT_LEAVE_GAME);

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
