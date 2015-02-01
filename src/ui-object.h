/**
 * \file ui-object.h
 * \brief Various object-related UI functions
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2007-9 Andi Sidwell, Chris Carr, Ed Graham, Erik Osheim
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

#ifndef UI_OBJECT_H
#define UI_OBJECT_H

void textui_cmd_ignore_menu(struct object *obj);
void textui_cmd_ignore(void);
void textui_cmd_toggle_ignore(void);
void textui_obj_examine(void);

#endif /* UI_OBJECT_H */
