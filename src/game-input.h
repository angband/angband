/**
 * \file game-input.h
 * \brief Ask for non-command input from the UI.
 *
 * Copyright (c) 2014 Nick McConnell
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

#ifndef INCLUDED_GAME_INPUT_H
#define INCLUDED_GAME_INPUT_H

#include "cmd-core.h"

/**
 * Bit flags for get_item() function
 */
#define USE_EQUIP     0x0001	/* Allow equip items */
#define USE_INVEN     0x0002	/* Allow inven items */
#define USE_FLOOR     0x0004	/* Allow floor items */
#define USE_QUIVER    0x0008	/* Allow quiver items */
#define IS_HARMLESS   0x0010	/* Ignore generic warning inscriptions */
#define SHOW_PRICES   0x0020	/* Show item prices in item lists */
#define SHOW_FAIL     0x0040 	/* Show device failure in item lists */
#define SHOW_QUIVER   0x0080	/* Show quiver summary when in inventory */
#define SHOW_EMPTY    0x0100	/* Show empty slots in equipment display */
#define QUIVER_TAGS   0x0200	/* 0-9 are quiver slots when selecting */
#define SHOW_RECHARGE 0x0400	/* Show item recharge failure in item lists */
#define SHOW_THROWING 0x0800	/* Show inventory/quiver/floor throwables*/


extern bool (*get_string_hook)(const char *prompt, char *buf, size_t len);
extern int (*get_quantity_hook)(const char *prompt, int max);
extern bool (*get_check_hook)(const char *prompt);
extern bool (*get_com_hook)(const char *prompt, char *command);
extern bool (*get_rep_dir_hook)(int *dir, bool allow_none);
extern bool (*get_aim_dir_hook)(int *dir);
extern int (*get_spell_from_book_hook)(struct player *p, const char *verb,
	struct object *book, const char *error,
	bool (*spell_filter)(const struct player *p, int spell));
extern int (*get_spell_hook)(struct player *p, const char *verb,
	item_tester book_filter, cmd_code cmd, const char *error,
	bool (*spell_filter)(const struct player *p, int spell));
extern bool (*get_item_hook)(struct object **choice, const char *pmt,
							 const char *str, cmd_code cmd, item_tester tester,
							 int mode);
extern bool (*get_curse_hook)(int *choice, struct object *obj,
							  char *dice_string);
extern int (*get_effect_from_list_hook)(const char *prompt,
	struct effect *effect, int count, bool allow_random);
extern bool (*confirm_debug_hook)(void);
extern void (*get_panel_hook)(int *min_y, int *min_x, int *max_y, int *max_x);
extern bool (*panel_contains_hook)(unsigned int y, unsigned int x);
extern bool (*map_is_visible_hook)(void);

bool get_string(const char *prompt, char *buf, size_t len);
int get_quantity(const char *prompt, int max);
bool get_check(const char *prompt);
bool get_com(const char *prompt, char *command);
bool get_rep_dir(int *dir, bool allow_none);
bool get_aim_dir(int *dir);
int get_spell_from_book(struct player *p, const char *verb,
	struct object *book, const char *error,
	bool (*spell_filter)(const struct player *p, int spell));
int get_spell(struct player *p, const char *verb,
	item_tester book_filter, cmd_code cmd, const char *error,
	bool (*spell_filter)(const struct player *p, int spell));
bool get_item(struct object **choice, const char *pmt, const char *str,
			  cmd_code cmd, item_tester tester, int mode);
bool get_curse(int *choice, struct object *obj, char *dice_string);
int get_effect_from_list(const char *prompt, struct effect *effect, int count,
	bool allow_random);
void get_panel(int *min_y, int *min_x, int *max_y, int *max_x);
bool confirm_debug(void);
bool panel_contains(unsigned int y, unsigned int x);
bool map_is_visible(void);

#endif /* INCLUDED_GAME_INPUT_H */
