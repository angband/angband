/**
 * \file game-input.c
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

#include "angband.h"
#include "cmd-core.h"
#include "game-input.h"
#include "player.h"

bool (*get_string_hook)(const char *prompt, char *buf, size_t len);
int (*get_quantity_hook)(const char *prompt, int max);
bool (*get_check_hook)(const char *prompt);
bool (*get_com_hook)(const char *prompt, char *command);
bool (*get_rep_dir_hook)(int *dir, bool allow_none);
bool (*get_aim_dir_hook)(int *dir);
int (*get_spell_from_book_hook)(struct player *p, const char *verb,
	struct object *book, const char *error,
	bool (*spell_filter)(const struct player *p, int spell));
int (*get_spell_hook)(struct player *p, const char *verb,
	item_tester book_filter, cmd_code cmd, const char *error,
	bool (*spell_filter)(const struct player *p, int spell));
bool (*get_item_hook)(struct object **choice, const char *pmt, const char *str,
					  cmd_code cmd, item_tester tester, int mode);
bool (*get_curse_hook)(int *choice, struct object *obj, char *dice_string);
int (*get_effect_from_list_hook)(const char* prompt,
	struct effect *effect, int count, bool allow_random);
bool (*confirm_debug_hook)(void);
void (*get_panel_hook)(int *min_y, int *min_x, int *max_y, int *max_x);
bool (*panel_contains_hook)(unsigned int y, unsigned int x);
bool (*map_is_visible_hook)(void);
void (*view_abilities_hook)(struct player_ability *ability_list,
							int num_abilities);

/**
 * Prompt for a string from the user.
 *
 * \param prompt is the prompt to the user, and should take the form "Prompt: "
 * \param buf is the user string, and the value passed in is the default
 * \param len is the length of buf
 * \return whether the user accepted the entered value or escaped
 */
bool get_string(const char *prompt, char *buf, size_t len)
{
	/* Ask the UI for it */
	if (get_string_hook)
		return get_string_hook(prompt, buf, len);
	else
		return false;
}

/**
 * Request a quantity from the user
 *
 * \param prompt is the prompt to the user, and should take the form "Prompt: "
 * \param max is the maximum value to accept
 * \return the quantity
 */
int get_quantity(const char *prompt, int max)
{
	/* Ask the UI for it */
	if (get_quantity_hook)
		return get_quantity_hook(prompt, max);
	else
		return 0;
}

/**
 * Verify something with the user
 *
 * \param prompt is the prompt to the user, and should take the form "Query? "
 * \return whether the user answered "y"
 *
 * get_check_hook() should be set to a function which asks the user for a
 * "y/n" answer
 */
bool get_check(const char *prompt)
{
	/* Ask the UI for it */
	if (get_check_hook)
		return get_check_hook(prompt);
	else
		return false;
}

/**
 * Prompts for a keypress
 *
 * \param prompt is the prompt to the user, and should take the form "Command: "
 * \param command stores the keypress
 * \return whether the user accepted the entered value or escaped
 */
bool get_com(const char *prompt, char *command)
{
	/* Ask the UI for it */
	if (get_com_hook)
		return get_com_hook(prompt, command);
	else
		return false;
}


/**
 * Request a "movement" direction from the user.
 *
 * \param dir is a pointer to an integer representing the chosen direction
 * \param allow_none can be set to true to allow the null direction
 * \return true if a direction was chosen, otherwise return false.
 */
bool get_rep_dir(int *dir, bool allow_none)
{
	/* Ask the UI for it */
	if (get_rep_dir_hook)
		return get_rep_dir_hook(dir, allow_none);
	else
		return false;
}

/**
 * Get an "aiming" direction from the user.
 *
 * \param dir is a pointer to an integer representing the chosen direction
 * \return true if a direction was chosen, otherwise return false.
 */
bool get_aim_dir(int *dir)
{
	/* Ask the UI for it */
	if (get_aim_dir_hook)
		return get_aim_dir_hook(dir);
	else
		return false;
}

/**
 * Get a spell from a specified book.
 */
int get_spell_from_book(struct player *p, const char *verb,
		struct object *book, const char *error,
		bool (*spell_filter)(const struct player *p, int spell))
{
	/* Ask the UI for it */
	if (get_spell_from_book_hook) {
		return get_spell_from_book_hook(p, verb, book, error,
			spell_filter);
	}
	return -1;
}

/**
 * Get a spell from the player.
 */
int get_spell(struct player *p, const char *verb,
		item_tester book_filter, cmd_code cmd, const char *error,
		bool (*spell_filter)(const struct player *p, int spell))
{
	/* Ask the UI for it */
	if (get_spell_hook) {
		return get_spell_hook(p, verb, book_filter, cmd, error,
			spell_filter);
	}
	return -1;
}

/**
 * Let the user select an object, save its address
 *
 * \param choice is the chosen object
 * \param pmt is the prompt to the player
 * \param str is the message if no valid item is available
 * \param cmd is the command (if any) the request is called from
 * \param tester is the function (if any) used to test for valid objects
 * \param mode gives more information on where the object can be chosen from
 *
 * If a legal item is selected , we save it in "obj" and return true.
 * If no item is available, we do nothing to "obj", and we display a
 *   warning message, using "str" if available, and return false.
 * If no item is selected, we do nothing to "obj", and return false.
 */
bool get_item(struct object **choice, const char *pmt, const char *str,
			  cmd_code cmd, item_tester tester, int mode)
{
	/* Ask the UI for it */
	if (get_item_hook)
		return get_item_hook(choice, pmt, str, cmd, tester, mode);
	else
		return false;
}

/**
 * Get a curse from an object
 */
bool get_curse(int *choice, struct object *obj, char *dice_string)
{
	/* Ask the UI for it */
	if (get_curse_hook)
		return get_curse_hook(choice, obj, dice_string);
	else
		return false;
}

/**
 * Select an effect from a list.
 * \param prompt is the prompt to present to the user.  May be NULL to use
 * a default prompt.
 * \param effect is the pointer to the first effect in the linked list.
 * \param count is the number of effects in the list.  If count is -1, use
 * all of the effects in the list.
 * \param allow_random if true, present an additional option which will
 * select a random effect from the list.  If false, only present the options
 * corresponding to the effects in the list.
 * \return the index of the selected item in the list, -2 if the user selected
 * the random option enabled by allow_random, or -1 to indicate a canceled or
 * invalid selection
 */
int get_effect_from_list(const char *prompt, struct effect *effect, int count,
	bool allow_random)
{
	/* Ask the UI for it */
	if (get_effect_from_list_hook) {
		return get_effect_from_list_hook(prompt, effect, count,
			allow_random);
	}
	/*
	 * If there's no UI implementation but a random selection is allowed,
	 * use that.
	 */
	return (allow_random) ? -2 : -1;
}

/**
 * Confirm whether to enable the debugging commands.
 */
bool confirm_debug(void)
{
	/* Use a UI-specific method. */
	if (confirm_debug_hook) {
		return confirm_debug_hook();
	}

	/* Otherwise, use a generic procedure.  First, mention effects. */
	msg("You are about to use the dangerous, unsupported, debug commands!");
	msg("Your machine may crash, and your savefile may become corrupted!");
	event_signal(EVENT_MESSAGE_FLUSH);

	/* Then verify. */
	return get_check("Are you sure you want to use the debug commands? ");
}

/**
 * Get the borders of the area the player can see (the "panel")
 */
void get_panel(int *min_y, int *min_x, int *max_y, int *max_x)
{
	/* Ask the UI for it */
	if (get_panel_hook)
		get_panel_hook(min_y, min_x, max_y, max_x);
}

/**
 * Check to see if a map grid is in the panel
 */
bool panel_contains(unsigned int y, unsigned int x)
{
	/* Ask the UI for it */
	if (panel_contains_hook)
		return panel_contains_hook(y, x);
	else
		return true;
}

/**
 * Check to see if the map is currently shown
 */
bool map_is_visible(void)
{
	/* Ask the UI for it */
	if (map_is_visible_hook)
		return map_is_visible_hook();
	else
		return true;
}

/**
 * Browse player abilities
 */
void view_ability_menu(struct player_ability *ability_list,
					   int num_abilities)
{
	/* Ask the UI for it */
	if (view_abilities_hook)
		view_abilities_hook(ability_list, num_abilities);
}
