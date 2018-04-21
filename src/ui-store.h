/**
 * \file ui-store.h
 * \brief Store UI
 *
 * Copyright (c) 1997 Robert A. Koeneke, James E. Wilson, Ben Harrison
 * Copyright (c) 1998-2014 Angband developers
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

#ifndef INCLUDED_UI_STORE_H
#define INCLUDED_UI_STORE_H


void textui_store_knowledge(int n);
void enter_store(game_event_type type, game_event_data *data, void *user);
void use_store(game_event_type type, game_event_data *data, void *user);
void leave_store(game_event_type type, game_event_data *data, void *user);

#endif /* INCLUDED_UI_STORE_H */
