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
 * Play Angband
 */
void play_game(bool new_game)
{
	/* Load a savefile or birth a character, or both */
	start_game(new_game);

	/* Get commands from the user, then process the game world until the
	 * command queue is empty and a new player command is needed */
	while (!player->is_dead && player->upkeep->playing) {
		cmd_get_hook(CMD_GAME);
		run_game_loop();
	}

	/* Close game on death or quitting */
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
