/**
 * \file source.c
 * \brief Type that allows various different origins for an effect
 *
 * Copyright (c) 2016 Andi Sidwell
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

#include "source.h"

struct source source_none(void)
{
	struct source src;
	src.what = SRC_NONE;
	return src;
}

struct source source_trap(struct trap *trap)
{
	struct source src;
	src.what = SRC_TRAP;
	src.which.trap = trap;
	return src;
}

struct source source_monster(int who)
{
	struct source src;
	src.what = SRC_MONSTER;
	src.which.monster = who;
	return src;
}

struct source source_player(void)
{
	struct source src = { SRC_PLAYER };
	return src;
}

struct source source_object(struct object *object)
{
	struct source src;
	src.what = SRC_OBJECT;
	src.which.object = object;
	return src;
}

struct source source_chest_trap(struct chest_trap *chest_trap)
{
	struct source src;
	src.what = SRC_CHEST_TRAP;
	src.which.chest_trap = chest_trap;
	return src;
}
