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

bool (*get_string_hook)(const char *prompt, char *buf, size_t len);
int (*get_quantity_hook)(const char *prompt, int max);
bool (*get_check_hook)(const char *prompt);
bool (*get_com_hook)(const char *prompt, char *command);
bool (*get_rep_dir_hook)(int *dir, bool allow_none);
bool (*get_aim_dir_hook)(int *dir);
int (*get_spell_from_book_hook)(const char *verb, struct object *book,
								const char *error,
								bool (*spell_filter)(int spell));
int (*get_spell_hook)(const char *verb, item_tester book_filter, cmd_code cmd,
					  const char *error, bool (*spell_filter)(int spell));
bool (*get_item_hook)(struct object **choice, const char *pmt, const char *str,
					  cmd_code cmd, item_tester tester, int mode);
void (*get_panel_hook)(int *min_y, int *min_x, int *max_y, int *max_x);
bool (*panel_contains_hook)(unsigned int y, unsigned int x);
bool (*map_is_visible_hook)(void);

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
int get_spell_from_book(const char *verb, struct object *book,
		const char *error, bool (*spell_filter)(int spell))
{
	/* Ask the UI for it */
	if (get_spell_from_book_hook)
		return get_spell_from_book_hook(verb, book, error, spell_filter);
	else
		return -1;
}

/**
 * Get a spell from the player.
 */
int get_spell(const char *verb, item_tester book_filter,
						cmd_code cmd, const char *error,
						bool (*spell_filter)(int spell))
{
	/* Ask the UI for it */
	if (get_spell_hook)
		return get_spell_hook(verb, book_filter, cmd, error, spell_filter);
	else
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
