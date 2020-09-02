/**
 * \file mon-list.h
 * \brief Monster list construction.
 *
 * Copyright (c) 1997-2007 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2013 Ben Semmler
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

#ifndef MONSTER_LIST_H
#define MONSTER_LIST_H

#include "angband.h"

typedef enum monster_list_section_e {
	MONSTER_LIST_SECTION_LOS = 0,
	MONSTER_LIST_SECTION_ESP,
	MONSTER_LIST_SECTION_MAX,
} monster_list_section_t;

typedef struct monster_list_entry_s {
	struct monster_race *race;
	u16b count[MONSTER_LIST_SECTION_MAX];
	u16b asleep[MONSTER_LIST_SECTION_MAX];
	s16b dx[MONSTER_LIST_SECTION_MAX], dy[MONSTER_LIST_SECTION_MAX];
	byte attr;
} monster_list_entry_t;

typedef struct monster_list_s {
	monster_list_entry_t *entries;
	size_t entries_size;
	u16b distinct_entries;
	s32b creation_turn;
	bool sorted;
	u16b total_entries[MONSTER_LIST_SECTION_MAX];
	u16b total_monsters[MONSTER_LIST_SECTION_MAX];
} monster_list_t;

monster_list_t *monster_list_new(void);
void monster_list_free(monster_list_t *list);
void monster_list_init(void);
void monster_list_finalize(void);
monster_list_t *monster_list_shared_instance(void);
void monster_list_reset(monster_list_t *list);
void monster_list_collect(monster_list_t *list);
int monster_list_standard_compare(const void *a, const void *b);
int monster_list_compare_exp(const void *a, const void *b);
void monster_list_sort(monster_list_t *list,
					   int (*compare)(const void *, const void *));
byte monster_list_entry_line_color(const monster_list_entry_t *entry);

#endif /* MONSTER_LIST_H */
