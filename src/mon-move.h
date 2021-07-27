/**
 * \file mon-move.h
 * \brief Monster movement
 *
 * Copyright (c) 1997 Ben Harrison, David Reeve Sward, Keldon Jones.
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
#ifndef MONSTER_MOVE_H
#define MONSTER_MOVE_H


enum monster_stagger {
	 NO_STAGGER = 0,
	 CONFUSED_STAGGER = 1,
	 INNATE_STAGGER = 2
};

bool multiply_monster(const struct monster *mon);
void process_monsters(int minimum_energy);
void reset_monsters(void);
void restore_monsters(void);

#endif /* !MONSTER_MOVE_H */
