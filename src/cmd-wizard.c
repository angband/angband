/**
 * \file cmd-wizard.c
 * \brief Implements debug commands in Angband 4's command system.
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
#include "cmds.h"
#include "effects.h"
#include "player-calcs.h"
#include "player-timed.h"


/**
 * Instantly cure the player of everything (CMD_WIZ_CURE_ALL).  Takes no
 * arguments from cmd.
 */
void do_cmd_wiz_cure_all(struct command *cmd)
{
	int i;

	/* Remove curses */
	for (i = 0; i < player->body.count; i++) {
		if (player->body.slots[i].obj &&
				player->body.slots[i].obj->curses) {
			mem_free(player->body.slots[i].obj->curses);
			player->body.slots[i].obj->curses = NULL;
		}
	}

	/* Restore stats */
	for (i = 0; i < STAT_MAX; i++) {
		effect_simple(EF_RESTORE_STAT, source_player(), "0", i,
			0, 0, 0, 0, NULL);
	}

	/* Restore the level */
	effect_simple(EF_RESTORE_EXP, source_none(), "0", 0, 0, 0, 0, 0, NULL);

	/* Heal the player */
	player->chp = player->mhp;
	player->chp_frac = 0;

	/* Restore mana */
	player->csp = player->msp;
	player->csp_frac = 0;

	/* Cure stuff */
	(void) player_clear_timed(player, TMD_BLIND, true);
	(void) player_clear_timed(player, TMD_CONFUSED, true);
	(void) player_clear_timed(player, TMD_POISONED, true);
	(void) player_clear_timed(player, TMD_AFRAID, true);
	(void) player_clear_timed(player, TMD_PARALYZED, true);
	(void) player_clear_timed(player, TMD_IMAGE, true);
	(void) player_clear_timed(player, TMD_STUN, true);
	(void) player_clear_timed(player, TMD_CUT, true);
	(void) player_clear_timed(player, TMD_SLOW, true);
	(void) player_clear_timed(player, TMD_AMNESIA, true);

	/* No longer hungry */
	player_set_timed(player, TMD_FOOD, PY_FOOD_FULL - 1, false);

	/* Flag what needs to be updated or redrawn */
	player->upkeep->update |= PU_TORCH | PU_UPDATE_VIEW | PU_MONSTERS;
	player->upkeep->redraw |= PR_BASIC | PR_EXTRA | PR_MAP | PR_EQUIP;

	/* Give the player some feedback */
	msg("You feel *much* better!");
}
