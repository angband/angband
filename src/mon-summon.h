/**
 * \file mon-summon.h
 * \brief Monster summoning

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

#ifndef MONSTER_SUMMON_H
#define MONSTER_SUMMON_H

#include "monster.h"

/* Flags for "summon_specific()"
 * (need better handling - NRM) */
enum summon_flag {
	S_ANY = 0,
    S_ANIMAL = 11,
    S_SPIDER = 12,
    S_HOUND = 13,
    S_HYDRA = 14,
    S_AINU = 15,
    S_DEMON = 16,
    S_UNDEAD = 17,
    S_DRAGON = 18,
    S_HI_DEMON = 26,
    S_HI_UNDEAD = 27,
    S_HI_DRAGON = 28,
    S_WRAITH = 31,
    S_UNIQUE = 32,
    S_KIN = 33,
    S_MONSTER = 41,
    S_MONSTERS = 42,
};

/** Variables **/
extern wchar_t summon_kin_type;		/* Hack -- See summon_specific() */


/** Functions **/
int summon_message_type(int summon_type);
int summon_specific(int y1, int x1, int lev, int type, int delay);

#endif /* MONSTER_SUMMON_H */
