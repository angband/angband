/**
 * \file ui-game.h
 * \brief Game management for the traditional text UI
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2015 Nick McConnell
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

#ifndef INCLUDED_UI_GAME_H
#define INCLUDED_UI_GAME_H

#include "cmd-core.h"
#include "game-event.h"

extern bool arg_wizard;
extern char savefile[1024];
extern void (*reinit_hook)(void);

void cmd_init(void);
unsigned char cmd_lookup_key(cmd_code lookup_cmd, int mode);
unsigned char cmd_lookup_key_unktrl(cmd_code lookup_cmd, int mode);
cmd_code cmd_lookup(unsigned char key, int mode);
size_t cmd_list_lookup_by_name(const char *name);
void textui_process_command(void);
errr textui_get_cmd(cmd_context context);
void check_for_player_interrupt(game_event_type type, game_event_data *data,
								void *user);
void play_game(bool new_game);
void savefile_set_name(const char *fname, bool make_safe, bool strip_suffix);
void save_game(void);
bool save_game_checked(void);
void close_game(bool prompt_failed_save);

#endif /* INCLUDED_UI_GAME_H */
