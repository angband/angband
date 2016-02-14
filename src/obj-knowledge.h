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

bool player_knows_brand(struct player *p, struct brand *b);
bool player_knows_slay(struct player *p, struct slay *s);
bool player_knows_ego(struct player *p, struct ego_item *ego);
void player_know_object(struct player *p, struct object *obj);
void player_learn_flag(struct player *p, int flag);
void player_learn_mod(struct player *p, int mod);
void player_learn_element(struct player *p, int element);
void player_learn_brand(struct player *p, struct brand *b);
void player_learn_slay(struct player *p, struct slay *s);
void player_learn_ac(struct player *p);
void player_learn_to_a(struct player *p);
void player_learn_to_h(struct player *p);
void player_learn_to_d(struct player *p);
void player_learn_dice(struct player *p);

void equip_learn_on_defend(struct player *p);
void missile_learn_on_ranged_attack(struct player *p, struct object *obj);
void equip_learn_on_ranged_attack(struct player *p);
void equip_learn_on_melee_attack(struct player *p);
void equip_learn_flag(struct player *p, int flag);
void equip_learn_element(struct player *p, int element);
void equip_learn_after_time(struct player *p);
void object_learn_on_wield(struct player *p, struct object *obj);
