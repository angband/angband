/**
 * \file ui-input.h
 * \brief Some high-level UI functions, inkey()
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

#ifndef INCLUDED_UI_INPUT_H
#define INCLUDED_UI_INPUT_H

#include "cmd-core.h"
#include "game-event.h"
#include "ui-event.h"
#include "ui-term.h"

/**
 * Holds a generic command.  If hook is not NULL, that function
 * will be called.  Otherwise, if cmd is not CMD_NULL, that command
 * will be pushed to the game.  If prereq is not NULL, it is tested
 * before pushing cmd or invoking hook.  If cmd is CMD_NULL and hook
 * is NULL, the command is an access point to a nested set of commands.
 * In that case, a positive value for nested_keymap sets the mapping
 * of keys to the nested commands; otherwise, those commands go through
 * the same lookup as directly entered commands.  When nested_keymap is
 * positive, nested_prompt is the prompt to solicit the key and should
 * be non-NULL for the commands to be activated.  nested_error is the
 * error message displayed if the entered key isn't recognized and may
 * be NULL.  nested_name is used to look up the struct command_list for
 * presentation of the nested commands through a menu.  nested_cached_idx
 * is for an optimization so that lookup only has to be done once;
 * initialize it to -1 for an access point to nested commands.
 */
struct cmd_info
{
	const char *desc;
	keycode_t key[2];
	cmd_code cmd;
	void (*hook)(void);
	bool (*prereq)(void);
	int nested_keymap;
	const char *nested_prompt;
	const char *nested_error;
	const char *nested_name;
	int nested_cached_idx;
};

/**
 * A categorised list of all the command lists.
 */
struct command_list
{
	const char *name;
	struct cmd_info *list;
	size_t len;
	int menu_level;
	int keymap;
};

#define SCAN_INSTANT ((uint32_t) -1)
#define SCAN_OFF 0

extern struct cmd_info cmd_item[];
extern struct cmd_info cmd_action[];
extern struct cmd_info cmd_item_manage[];
extern struct cmd_info cmd_info[];
extern struct cmd_info cmd_util[];
extern struct cmd_info cmd_hidden[];
extern struct command_list cmds_all[];

extern struct keypress *inkey_next;
extern uint32_t inkey_scan;
extern bool inkey_flag;
extern uint16_t lazymove_delay;
extern bool msg_flag;
extern bool arg_force_name;

void flush(game_event_type unused, game_event_data *data, void *user);
ui_event inkey_ex(void);
void anykey(void);
struct keypress inkey(void);
ui_event inkey_m(void);
void display_message(game_event_type unused, game_event_data *data, void *user);
void bell_message(game_event_type unused, game_event_data *data, void *user);
void message_flush(game_event_type unused, game_event_data *data, void *user);
void clear_from(int row);
bool askfor_aux_keypress(char *buf, size_t buflen, size_t *curs, size_t *len,
						 struct keypress keypress, bool firsttime);
int askfor_aux_mouse(char *buf, size_t buflen, size_t *curs, size_t *len,
	struct mouseclick mouse, bool firsttime);
bool askfor_aux(char *buf, size_t len, bool (*keypress_h)(char *, size_t, size_t *, size_t *, struct keypress, bool));
bool askfor_aux_ext(char *buf, size_t len,
	bool (*keypress_h)(char *, size_t, size_t *, size_t *, struct keypress, bool),
	int (*mouse_h)(char *, size_t, size_t *, size_t *, struct mouseclick, bool));
bool get_character_name(char *buf, size_t buflen);
char get_char(const char *prompt, const char *options, size_t len,
			  char fallback);
extern bool (*get_file)(const char *suggested_name, char *path, size_t len);
bool get_com_ex(const char *prompt, ui_event *command);
void pause_line(struct term *tm);
void textui_input_init(void);
ui_event textui_get_command(int *count);
bool key_confirm_command(unsigned char c);
bool textui_process_key(struct keypress kp, unsigned char *c, int count);

#endif /* INCLUDED_UI_INPUT_H */
