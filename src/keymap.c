/*
 * File: keymap.c
 * Purpose: Keymap handling
 *
 * Copyright (c) [yyyy] [author list]
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


/*
 * Keymaps for each "mode" associated with each keypress.
 */
char *keymap_act[KEYMAP_MODES][256];

