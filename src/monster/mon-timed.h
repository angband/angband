/*
 * File: mon-timed.h
 * Purpose: Structures and functions for monster timed effects.
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

#ifndef MONSTER_TIMED_H
#define MONSTER_TIMED_H

#include "angband.h"

/** Constants **/

/* Monster Timed Effects */
enum {
	MON_TMD_SLEEP = 0,
	MON_TMD_STUN,
	MON_TMD_CONF,
	MON_TMD_FEAR,
	MON_TMD_SLOW,
	MON_TMD_FAST,

	MON_TMD_MAX
};

/** Macros **/

/* Flags for the monster timed functions */
#define MON_TMD_FLG_NOTIFY		0x01 /* Give notification */
#define MON_TMD_MON_SOURCE		0x02 /* Monster is causing the damage */
#define MON_TMD_FLG_NOMESSAGE	0x04 /* Never show a message */
#define MON_TMD_FLG_NOFAIL		0x08 /* Never fail */

/** Structures **/

/** Variables **/


/** Functions **/
bool mon_inc_timed(int m_idx, int ef_idx, int timer, u16b flag);
bool mon_dec_timed(int m_idx, int ef_idx, int timer, u16b flag);
bool mon_clear_timed(int m_idx, int ef_idx, u16b flag);


#endif /* MONSTER_TIMED_H */
