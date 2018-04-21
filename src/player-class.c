/**
 * \file player-class.c
 * \brief Player classes
 *
 * Copyright (c) 2011 elly+angband@leptoquark.net. See COPYING.
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


#include "player.h"

struct player_class *player_id2class(guid id)
{
	struct player_class *c;
	for (c = classes; c; c = c->next)
		if (guid_eq(c->cidx, id))
			break;
	return c;
}
