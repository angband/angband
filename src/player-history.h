/**
 * \file player-history.h
 * \brief Character auto-history creation and management
 *
 * Copyright (c) 2007 J.D. White
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

#ifndef HISTORY_H
#define HISTORY_H

#include "angband.h"

/**
 * History message types
 */
enum {
	#define HIST(a, b) HIST_##a,
	#include "list-history-types.h"
	#undef HIST

	HIST_MAX
};


#define HIST_SIZE			FLAG_SIZE(HIST_MAX)

#define hist_has(f, flag)	flag_has_dbg(f, HIST_SIZE, flag, #f, #flag)
#define hist_on(f, flag)	flag_on_dbg(f, HIST_SIZE, flag, #f, #flag)
#define hist_off(f, flag)	flag_off(f, HIST_SIZE, flag)
#define hist_wipe(f)		flag_wipe(f, HIST_SIZE)
#define hist_copy(f1, f2)	flag_copy(f1, f2, HIST_SIZE)

/**
 * Player history table
 */
struct history_info {
	bitflag type[HIST_SIZE];/* Kind of history item */
	s16b dlev;				/* Dungeon level when this item was recorded */
	s16b clev;				/* Character level when this item was recorded */
	byte a_idx;				/* Artifact this item relates to */
	s32b turn;				/* Turn this item was recorded on */
	char event[80];			/* The text of the item */
};

void history_clear(struct player *p);
bool history_add_full(struct player *p,
		bitflag *type,
		int aidx,
		int dlev,
		int clev,
		int turn,
		const char *text);
bool history_add(struct player *p, const char *text, int type);
bool history_is_artifact_known(struct player *p, const struct artifact *artifact);
void history_find_artifact(struct player *p, const struct artifact *artifact);
void history_lose_artifact(struct player *p, const struct artifact *artifact);
void history_unmask_unknown(struct player *p);
size_t history_get_list(struct player *p, struct history_info **list);

#endif /* !HISTORY_H */
