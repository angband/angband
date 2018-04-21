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

/** Constants **/

/**
 * Monster message constants
 */
enum mon_messages {
	#define MON_MSG(x, s) MON_MSG_##x,
	#include "list-mon-message.h"
	#undef MON_MSG
};

enum {
	MON_DELAY_TAG_DEFAULT = 0,
	MON_DELAY_TAG_DEATH,
};

/**
 * Maxinum number of stacked monster messages
 */
#define MAX_STORED_MON_MSG		200
#define MAX_STORED_MON_CODES	400

enum mon_msg_flags {
	MON_MSG_FLAG_HIDDEN = 0x01, /* What is this? - NRM */
	MON_MSG_FLAG_OFFSCREEN = 0x02,
	MON_MSG_FLAG_INVISIBLE = 0x04
};

/** Structures **/

/**
 * A stacked monster message entry
 */
typedef struct monster_race_message
{
	struct monster_race *race;	/* The race of the monster */
	byte mon_flags;		/* Flags */
 	int  msg_code;		/* The coded message */
	byte mon_count;		/* How many monsters triggered this message */
	bool delay;			/* Should this message be put off to the end */
	byte delay_tag;		/* To group delayed messages for better presentation */
} monster_race_message;

typedef struct monster_message_history
{
	struct monster *mon;	/* The monster */
	int message_code;		/* The coded message */
} monster_message_history;


/** Variables **/
extern monster_race_message *mon_msg;
extern monster_message_history *mon_message_hist;

/** Functions **/
void message_pain(struct monster *m, int dam);
bool add_monster_message(const char *mon_name, struct monster *m, int msg_code,
						 bool delay);
void flush_all_monster_messages(void);

#endif /* MONSTER_MESSAGE_H */
