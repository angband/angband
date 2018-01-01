/**
 * \file mon-predicate.h
 * \brief Monster predicates
 *
 * Copyright (c) 1997-2007 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2017 Nick McConnell
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

/**
 * monster_predicate is a function pointer which tests a given monster to
 * see if the predicate in question is true.
 */
typedef bool (*monster_predicate)(const struct monster *mon);

bool monster_is_nonliving(const struct monster *mon);
bool monster_is_living(const struct monster *mon);
bool monster_is_destroyed(const struct monster *mon);
bool monster_passes_walls(const struct monster *mon);
bool monster_is_invisible(const struct monster *mon);
bool monster_is_not_invisible(const struct monster *mon);
bool monster_is_unique(const struct monster *mon);
bool monster_is_stupid(const struct monster *mon);
bool monster_is_smart(const struct monster *mon);
bool monster_is_evil(const struct monster *mon);
bool monster_is_powerful(const struct monster *mon);

bool monster_is_in_view(const struct monster *mon);
bool monster_is_visible(const struct monster *mon);
bool monster_is_camouflaged(const struct monster *mon);
bool monster_is_obvious(const struct monster *mon);
bool monster_is_mimicking(const struct monster *mon);


