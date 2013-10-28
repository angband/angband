/*
 * File: mon-msg.h
 * Purpose: Structures and functions for monster messages.
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

#include "angband.h"

/** Constants **/

/* The codified monster messages */
enum mon_messages {
	MON_MSG_NONE = 0,

	/* project_m */
	MON_MSG_DIE,
	MON_MSG_DESTROYED,
	MON_MSG_RESIST_A_LOT,
	MON_MSG_HIT_HARD,
	MON_MSG_RESIST,
	MON_MSG_IMMUNE,
	MON_MSG_RESIST_SOMEWHAT,
	MON_MSG_UNAFFECTED,
	MON_MSG_SPAWN,
	MON_MSG_HEALTHIER,
	MON_MSG_FALL_ASLEEP,
	MON_MSG_WAKES_UP,
	MON_MSG_CRINGE_LIGHT,
	MON_MSG_SHRIVEL_LIGHT,
	MON_MSG_LOSE_SKIN,
	MON_MSG_DISSOLVE,
	MON_MSG_CATCH_FIRE,
	MON_MSG_BADLY_FROZEN,
	MON_MSG_SHUDDER,
	MON_MSG_CHANGE,
	MON_MSG_DISAPPEAR,
	MON_MSG_MORE_DAZED,
	MON_MSG_DAZED,
	MON_MSG_NOT_DAZED,
	MON_MSG_MORE_CONFUSED,
	MON_MSG_CONFUSED,
	MON_MSG_NOT_CONFUSED,
	MON_MSG_MORE_SLOWED,
	MON_MSG_SLOWED,
	MON_MSG_NOT_SLOWED,
	MON_MSG_MORE_HASTED,
	MON_MSG_HASTED,
	MON_MSG_NOT_HASTED,
	MON_MSG_MORE_AFRAID,
	MON_MSG_FLEE_IN_TERROR,
	MON_MSG_NOT_AFRAID,
	MON_MSG_MORIA_DEATH,
	MON_MSG_DISENTEGRATES,
	MON_MSG_FREEZE_SHATTER,
	MON_MSG_MANA_DRAIN,
	MON_MSG_BRIEF_PUZZLE,
	MON_MSG_MAINTAIN_SHAPE,
	
	/* message_pain */
	MON_MSG_UNHARMED,
	MON_MSG_95,
	MON_MSG_75,
	MON_MSG_50,
	MON_MSG_35,
	MON_MSG_20,
	MON_MSG_10,
	MON_MSG_0,

	/* Always leave this at the end */
	MAX_MON_MSG
};


/* Maxinum number of stacked monster messages */
#define MAX_STORED_MON_MSG		200
#define MAX_STORED_MON_CODES	400



/** Macros **/

/** Structures **/
/*
 * A stacked monster message entry
 */
typedef struct monster_race_message
{
	s16b mon_race;		/* The race of the monster */
	byte mon_flags;		/* Flags: 0x01 means hidden monster, 0x02 means offscreen monster */
 	int  msg_code;		/* The coded message */
	byte mon_count;		/* How many monsters triggered this message */
	bool delay;			/* Should this message be put off to the end */
} monster_race_message;

typedef struct monster_message_history
{
	int monster_idx;	/* The monster */
	int message_code;		/* The coded message */
} monster_message_history;


/** Variables **/
monster_race_message *mon_msg;
monster_message_history *mon_message_hist;

/** Functions **/
void message_pain(int m_idx, int dam);
bool add_monster_message(const char *mon_name, int m_idx, int msg_code, bool delay);
void flush_all_monster_messages(void);

#endif /* MONSTER_MESSAGE_H */
