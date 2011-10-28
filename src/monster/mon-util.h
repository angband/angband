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

/*
 * Bit flags for the "monster_desc" function
 */
#define MDESC_OBJE      0x01    /* Objective (or Reflexive) */
#define MDESC_POSS      0x02    /* Possessive (or Reflexive) */
#define MDESC_IND1      0x04    /* Indefinites for hidden monsters */   
#define MDESC_IND2      0x08    /* Indefinites for visible monsters */   
#define MDESC_PRO1      0x10    /* Pronominalize hidden monsters */
#define MDESC_PRO2      0x20    /* Pronominalize visible monsters */ 
#define MDESC_HIDE      0x40    /* Assume the monster is hidden */
#define MDESC_SHOW      0x80    /* Assume the monster is visible */
#define MDESC_CAPITAL   0x100   /* Capitalise */


/** Macros **/

/** Structures **/

/** Variables **/
wchar_t summon_kin_type;		/* Hack -- See summon_specific() */


/** Functions **/
int lookup_monster(const char *name);
monster_base *lookup_monster_base(const char *name);
bool match_monster_bases(const monster_base *base, ...);
void plural_aux(char *name, size_t max);
void display_monlist(void);
void monster_desc(char *desc, size_t max, const monster_type *m_ptr, int mode);
void update_mon(int m_idx, bool full);
void update_monsters(bool full);
s16b monster_carry(struct monster *m, object_type *j_ptr);
void monster_swap(int y1, int x1, int y2, int x2);
int summon_specific(int y1, int x1, int lev, int type, int delay);
bool multiply_monster(const struct monster *m);
void become_aware(struct monster *m);
bool is_mimicking(struct monster *m);
void update_smart_learn(struct monster *m, struct player *p, int flag);

#endif /* MONSTER_UTILITIES_H */
