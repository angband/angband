/**
 * \file obj-knowledge.h
 * \brief Object knowledge
 *
 * Copyright (c) 2016 Nick McConnell
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

#include "angband.h"
#include "object.h"
#include "player.h"

void update_player_object_knowledge(struct player *p);
bool player_learn_flag(struct player *p, int flag);
bool player_learn_mod(struct player *p, int mod);
bool player_learn_element(struct player *p, int element);
bool player_learn_brand(struct player *p, struct brand *b);
bool player_learn_slay(struct player *p, struct slay *s);
bool player_learn_ac(struct player *p);
bool player_learn_to_a(struct player *p);
bool player_learn_to_h(struct player *p);
bool player_learn_to_d(struct player *p);
bool player_learn_dice(struct player *p);

