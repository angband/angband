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

enum rune_variety {
	RUNE_VAR_COMBAT,
	RUNE_VAR_FLAG,
	RUNE_VAR_MOD,
	RUNE_VAR_RESIST,
	RUNE_VAR_BRAND,
	RUNE_VAR_SLAY
};


enum combat_runes {
	COMBAT_RUNE_TO_A = 0,
	COMBAT_RUNE_TO_H,
	COMBAT_RUNE_TO_D,
	COMBAT_RUNE_MAX
};

struct rune {
	enum rune_variety variety;
	int index;
	const char *name;
};

bool player_knows_brand(struct player *p, struct brand *b);
bool player_knows_slay(struct player *p, struct slay *s);
bool player_knows_ego(struct player *p, struct ego_item *ego);
bool object_effect_is_known(const struct object *obj);
bool object_is_known_artifact(const struct object *obj);
bool object_is_in_store(const struct object *obj);
bool object_runes_known(const struct object *obj);
bool object_fully_known(const struct object *obj);
bool object_flag_is_known(const struct object *obj, int flag);
bool object_element_is_known(const struct object *obj, int element);

void object_set_base_known(struct object *obj);
void player_know_object(struct player *p, struct object *obj);
void update_player_object_knowledge(struct player *p);

void player_learn_everything(struct player *p);

void equip_learn_on_defend(struct player *p);
void equip_learn_on_ranged_attack(struct player *p);
void equip_learn_on_melee_attack(struct player *p);
void equip_learn_flag(struct player *p, int flag);
void equip_learn_element(struct player *p, int element);
void equip_learn_after_time(struct player *p);

void object_learn_unknown_rune(struct player *p, struct object *obj);
void object_learn_on_wield(struct player *p, struct object *obj);
void object_learn_on_use(struct player *p, struct object *obj);
void object_learn_brand(struct player *p, struct object *obj, struct brand *b);
void object_learn_slay(struct player *p, struct object *obj, struct slay *s);
void missile_learn_on_ranged_attack(struct player *p, struct object *obj);

bool easy_know(const struct object *obj);
bool object_flavor_is_aware(const struct object *obj);
bool object_flavor_was_tried(const struct object *obj);
void object_flavor_aware(struct object *obj);
void object_flavor_tried(struct object *obj);
