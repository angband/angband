/**
 * \file mon-attack.h
 * \brief Monster attacks
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

#ifndef MONSTER_ATTACK_H
#define MONSTER_ATTACK_H

int choose_attack_spell(bitflag *f, bool innate, bool non_innate);
bool make_ranged_attack(struct monster *mon);
bool check_hit(struct player *p, int power, int level, int accuracy);
bool check_hit_monster(struct monster *mon, int power, int level, int accuracy);
int adjust_dam_armor(int damage, int ac);
bool make_attack_normal(struct monster *mon, struct player *p);
bool monster_attack_monster(struct monster *mon, struct monster *t_mon);

#endif /* !MONSTER_ATTACK_H */
