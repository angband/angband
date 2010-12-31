/* player/race.c */

#include "externs.h"
#include "player/player.h"
#include "player/types.h"

struct player_race *player_id2race(guid id)
{
	struct player_race *r;
	for (r = races; r; r = r->next)
		if (guid_eq(r->ridx, id))
			break;
	return r;
}
