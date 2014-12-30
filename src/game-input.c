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

bool (*get_check_hook)(const char *prompt);

/**
 * Verify something with the user
 *
 * The prompt should take the form "Query? ", and get_check_hook() should be
 * set to a function which asks the user for a "y/n" answer
 */
bool get_check(const char *prompt)
{
	/* Ask the UI for it */
	return get_check_hook(prompt);
}

