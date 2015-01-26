/**
 * \file ui-context.h
 * \brief Show player and terrain context menus.
 *
 * Copyright (c) 2011 Brett Reid
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband license":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#ifndef UI_CONTEXT_H
#define UI_CONTEXT_H

#include "cave.h"
#include "ui-input.h"

int context_menu_player(int mx, int my);
int context_menu_cave(struct chunk *c, int y, int x, int adjacent, int mx,
					  int my);
int context_menu_object(struct object *obj);
int context_menu_command(int mx, int my);
void textui_process_click(ui_event e);
struct cmd_info *textui_action_menu_choose(void);

#endif /* UI_CONTEXT_H */
