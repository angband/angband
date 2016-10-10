/**
 * \file mon-msg.h
 * \brief Structures and functions for monster messages.
 *
 * Copyright (c) 1997-2007 Ben Harrison, James E. Wilson, Robert A. Koeneke
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

#ifndef MONSTER_MESSAGE_H
#define MONSTER_MESSAGE_H

#include "monster.h"

/**
 * Monster message constants
 */
enum mon_messages {
	#define MON_MSG(x, t, o, s) MON_MSG_##x,
	#include "list-mon-message.h"
	#undef MON_MSG
};

void message_pain(struct monster *m, int dam);
bool add_monster_message(struct monster *m, int msg_code, bool delay);
void show_monster_messages(void);

#endif /* MONSTER_MESSAGE_H */
