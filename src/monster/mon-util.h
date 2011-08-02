/*
 * File: mon-util.h
 * Purpose: Structures and functions for monster utilities.
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

#ifndef MONSTER_UTILITIES_H
#define MONSTER_UTILITIES_H

#include "angband.h"

/** Constants **/

/** Macros **/

/** Structures **/

/* 
 * Monster data for the visible monster list 
 */
typedef struct
{
	u16b count;		/* total number of this type visible */
	u16b asleep;		/* number asleep (not in LOS) */
	u16b los;		/* number in LOS */
	u16b los_asleep;	/* number asleep and in LOS */
	byte attr; /* attr to use for drawing */
} monster_vis; 


/** Variables **/
char summon_kin_type;		/* Hack -- See summon_specific() */


/** Functions **/
bool mon_inc_timed(int m_idx, int idx, int v, u16b flag);
bool mon_dec_timed(int m_idx, int idx, int v, u16b flag);
bool mon_clear_timed(int m_idx, int idx, u16b flag);
int lookup_monster(const char *name);
monster_base *lookup_monster_base(const char *name);
bool match_monster_bases(monster_base *base, ...);
void display_monlist(void);
void monster_desc(char *desc, size_t max, const monster_type *m_ptr, int mode);
void update_mon(int m_idx, bool full);
void update_monsters(bool full);
s16b monster_carry(struct monster *m, object_type *j_ptr);
void monster_swap(int y1, int x1, int y2, int x2);
bool summon_specific(int y1, int x1, int lev, int type, int delay);
bool multiply_monster(int m_idx);
void update_smart_learn(struct monster *m, struct player *p, int what);
void become_aware(int m_idx);
bool is_mimicking(int m_idx);
void plural_aux(char *name, size_t max);



#endif /* MONSTER_UTILITIES_H */
