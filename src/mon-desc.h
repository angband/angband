/**
 * \file mon-desc.h
 * \brief Monster description
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

#ifndef MONSTER_DESC_H
#define MONSTER_DESC_H

#include "monster.h"

/**
 * Bit flags for the "monster_desc" function
 */
#define MDESC_DEFAULT   0x00    /* "it" or "the kobold" */
#define MDESC_OBJE      0x01    /* Objective (or Reflexive) */
#define MDESC_POSS      0x02    /* Possessive (or Reflexive) */
#define MDESC_IND_HID   0x04    /* Indefinites for hidden monsters */
#define MDESC_IND_VIS   0x08    /* Indefinites for visible monsters */
#define MDESC_PRO_HID   0x10    /* Pronominalize hidden monsters */
#define MDESC_PRO_VIS   0x20    /* Pronominalize visible monsters */ 
#define MDESC_HIDE      0x40    /* Assume the monster is hidden */
#define MDESC_SHOW      0x80    /* Assume the monster is visible */
#define MDESC_CAPITAL   0x100   /* Capitalise */

/* "someone", "something", or "the kobold" at the start of a message */
#define MDESC_STANDARD  (MDESC_CAPITAL | MDESC_IND_HID | MDESC_PRO_HID)

 /* Reveal the full, indefinite name of a monster */
#define MDESC_DIED_FROM (MDESC_SHOW | MDESC_IND_VIS)

void plural_aux(char *name, size_t max);
void get_mon_name(char *output_name, size_t max,
				  const struct monster_race *race, int num);
void monster_desc(char *desc, size_t max, const struct monster *mon, int mode);

#endif /* MONSTER_DESC_H */
