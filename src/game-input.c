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

bool (*get_string_hook)(const char *prompt, char *buf, size_t len);
int (*get_quantity_hook)(const char *prompt, int max);
bool (*get_check_hook)(const char *prompt);
bool (*get_com_hook)(const char *prompt, char *command);
bool (*get_rep_dir_hook)(int *dir, bool allow_none);
bool (*get_aim_dir_hook)(int *dir);

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
	return get_string_hook(prompt, buf, len);
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
	return get_quantity_hook(prompt, max);
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
	return get_check_hook(prompt);
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
	return get_com_hook(prompt, command);
}


/**
 * Request a "movement" direction from the user.
 *
 * \param dir is a pointer to an integer representing the chosen direction
 * \param allow_none can be set to true to allow the null direction
 * \return TRUE if a direction was chosen, otherwise return FALSE.
 */
bool get_rep_dir(int *dir, bool allow_none)
{
	/* Ask the UI for it */
	return get_rep_dir_hook(dir, allow_none);
}

/**
 * Get an "aiming" direction from the user.
 *
 * \param dir is a pointer to an integer representing the chosen direction
 * \return TRUE if a direction was chosen, otherwise return FALSE.
 */
bool get_aim_dir(int *dir)
{
	/* Ask the UI for it */
	return get_aim_dir_hook(dir);
}
