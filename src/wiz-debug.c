/**
 * \file wiz-debug.c
 * \brief Implement miscellaneous debug mode functions
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
#include "player-timed.h"
#include "player-util.h"
#include "wizard.h"


/**
 * What happens when you cheat death.  Tsk, tsk.
 */
void wiz_cheat_death(void)
{
	/* Mark social class, reset age, if needed */
	player->age = 1;
	player->noscore |= NOSCORE_WIZARD;

	player->is_dead = false;

	/* Restore hit & spell points */
	player->chp = player->mhp;
	player->chp_frac = 0;
	player->csp = player->msp;
	player->csp_frac = 0;

	/* Healing */
	(void)player_clear_timed(player, TMD_BLIND, true);
	(void)player_clear_timed(player, TMD_CONFUSED, true);
	(void)player_clear_timed(player, TMD_POISONED, true);
	(void)player_clear_timed(player, TMD_AFRAID, true);
	(void)player_clear_timed(player, TMD_PARALYZED, true);
	(void)player_clear_timed(player, TMD_IMAGE, true);
	(void)player_clear_timed(player, TMD_STUN, true);
	(void)player_clear_timed(player, TMD_CUT, true);

	/* Prevent starvation */
	player_set_timed(player, TMD_FOOD, PY_FOOD_MAX - 1, false);

	/* Cancel recall */
	if (player->word_recall)
	{
		/* Message */
		msg("A tension leaves the air around you...");
		event_signal(EVENT_MESSAGE_FLUSH);

		/* Hack -- Prevent recall */
		player->word_recall = 0;
	}

	/* Cancel deep descent */
	if (player->deep_descent)
	{
		/* Message */
		msg("The air around you stops swirling...");
		event_signal(EVENT_MESSAGE_FLUSH);

		/* Hack -- Prevent recall */
		player->deep_descent = 0;
	}

	/* Note cause of death XXX XXX XXX */
	my_strcpy(player->died_from, "Cheating death", sizeof(player->died_from));

	/* Back to the town */
	dungeon_change_level(player, 0);
}
