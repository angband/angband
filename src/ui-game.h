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

/* These are the allowed modes of operation for play_game(). */
enum game_mode_type {
	GAME_LOAD,	/* try to load the game from savefile; if it does not
				exist or the character is dead, start a new
				game that'll be stored in that savefile */
	GAME_NEW,	/* always start a new game regardless of the state
				of the character in savefile; new game will
				be stored with the name in savefile unless it
				is empty */
	GAME_SELECT	/* have the player select with a menu what to do; if
				savefile is set and exists that will be the
				default option in the menu for loading; if
				savefile is not empty and does not exist,
				it'll be the file used if the player selects
				the menu option for a new game */
};

/*
 * This is an opaque pointer for enumerating the savefiles available to the
 * current player from the savefile directory.
 */
typedef struct savefile_getter_impl *savefile_getter;

/* Holds the information that can be gotten back from a savefile_getter. */
struct savefile_details {
	char *fnam;	/* holds the file name component of its path */
	char *desc;	/* holds the result from savefile_get_description() */
	size_t foff;	/* holds the offset in fnam to get past the
				player-specific prefix */
};

extern bool arg_wizard;
extern char savefile[1024];
extern char panicfile[1024];
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
void play_game(enum game_mode_type mode);
void savefile_set_name(const char *fname, bool make_safe, bool strip_suffix);
bool savefile_name_already_used(const char *fname, bool make_safe,
	bool strip_suffix);
void save_game(void);
bool save_game_checked(void);
void close_game(bool prompt_failed_save);

bool got_savefile(savefile_getter *pg);
bool got_savefile_dir(const savefile_getter g);
const struct savefile_details *get_savefile_details(const savefile_getter g);
void cleanup_savefile_getter(savefile_getter g);

#endif /* INCLUDED_UI_GAME_H */
