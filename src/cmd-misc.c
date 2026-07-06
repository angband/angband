/**
 * \file cmd-misc.c
 * \brief Deal with miscellaneous commands.
 *
 * Copyright (c) 2010 Andi Sidwell
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
#include "buildid.h"
#include "cave.h"
#include "cmd-core.h"
#include "cmds.h"
#include "game-input.h"
#include "init.h"
#include "mon-lore.h"
#include "mon-util.h"
#include "player-calcs.h"
#include "player-history.h"
#include "obj-util.h"
#include "target.h"


/**
 * Toggle wizard mode
 */
void do_cmd_wizard(void)
{
	/* Verify first time */
	if (!(player->noscore & NOSCORE_WIZARD)) {
		/* Mention effects */
		msg("You are about to enter 'wizard' mode for the very first time!");
		msg("This is a form of cheating, and your game will not be scored!");
		event_signal(EVENT_MESSAGE_FLUSH);

		/* Verify request */
		if (!get_check("Are you sure you want to enter wizard mode? "))
			return;

		/* Mark savefile */
		player->noscore |= NOSCORE_WIZARD;
	}

	/* Toggle mode */
	if (player->wizard) {
		player->wizard = false;
		msg("Wizard mode off.");
	} else {
		player->wizard = true;
		msg("Wizard mode on.");
	}

	/* Update monsters */
	player->upkeep->update |= (PU_MONSTERS);

	/* Redraw "title" */
	player->upkeep->redraw |= (PR_TITLE);
}

/**
 * Retire
 */
void do_cmd_retire(struct command *cmd)
{
	/* Treat retired character as dead to satisfy end of game logic. */
	player->is_dead = true;
	my_strcpy(player->died_from, "Retiring", sizeof(player->died_from));
}

/**
 * Record the player's thoughts as a note.
 *
 * This both displays the note back to the player and adds it to the game log.
 * If the note begins with "/" followed by specific strings, those will be
 * expanded when the history is viewed.  See history_expand_user_input()'s
 * documentation for details about that.
 */
void do_cmd_note(void)
{
	char note[80] = "";

	/* Read a line of input from the user */
	if (!get_string("Note: ", note, sizeof(note))) return;

	/* Ignore empty notes */
	if (!note[0] || (note[0] == ' ')) return;

	/* Add a history entry, with the user's text as is */
	history_add(player, note, HIST_USER_INPUT);

	/*
	 * Provide feedback with the note expanded, except for a "-- " prefix,
	 * as it will be when the history is displayed.
	 */
	msg(history_expand_user_input(note, player, NULL, 0, false));
}

#ifdef ALLOW_BORG

extern void do_cmd_borg(void);

/*
 * Verify use of "borg" mode
 */
void do_cmd_try_borg(void)
{
	/* Ask first time */
	if (!(player->noscore & NOSCORE_BORG))
	{
		/* Mention effects */
		msg("You are about to use the dangerous, unsupported, borg commands!");
		msg("Your machine may crash, and your savefile may become corrupted!");
		event_signal(EVENT_MESSAGE_FLUSH);

		/* Verify request */
		if (!get_check("Are you sure you want to use the borg commands? "))
			return;

		/* Mark savefile */
		player->noscore |= NOSCORE_BORG;
	}

	/* Okay */
	do_cmd_borg();
}

#endif /* ALLOW_BORG */
