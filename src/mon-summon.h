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

struct monster_base_list {
	struct monster_base *base;
	struct monster_base_list *next;
};

struct summon {
	struct summon *next;
	char *name;
	int message_type;
	bool unique_allowed;
	struct monster_base_list *bases;
	int race_flag;
	char *fallback_name;
	int fallback;
	char *desc;
};

/** Variables **/
extern struct monster_base *kin_base;
extern struct file_parser summon_parser;

/** Functions **/
int summon_name_to_idx(const char *name);
int summon_fallback_type(int summon_type);
int summon_message_type(int summon_type);
const char *summon_desc(int type);
int summon_specific(struct loc grid, int lev, int type, bool delay, bool call);
struct monster_race *select_shape(struct monster *mon, int type);

#endif /* MONSTER_SUMMON_H */
