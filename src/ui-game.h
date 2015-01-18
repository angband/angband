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

void cmd_init(void);
unsigned char cmd_lookup_key(cmd_code lookup_cmd, int mode);
unsigned char cmd_lookup_key_unktrl(cmd_code lookup_cmd, int mode);
cmd_code cmd_lookup(unsigned char key, int mode);
void textui_process_command(void);
errr textui_get_cmd(cmd_context context);

#endif /* INCLUDED_UI_GAME_H */
