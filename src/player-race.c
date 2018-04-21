/**
 * \file player-race.c
 * \brief Player races
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

struct player_race *player_id2race(guid id)
{
	struct player_race *r;
	for (r = races; r; r = r->next)
		if (guid_eq(r->ridx, id))
			break;
	return r;
}
