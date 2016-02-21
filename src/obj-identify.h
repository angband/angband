/**
 * \file obj-identify.h
 * \brief Object identification and knowledge routines
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2009 Brian Bull
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

#ifndef OBJECT_IDENTIFY_H
#define OBJECT_IDENTIFY_H

/**
 * Pseudo-ID markers.
 */
typedef enum
{
	INSCRIP_NULL = 0,            /*!< No pseudo-ID status */
	INSCRIP_STRANGE = 1,         /*!< Item that has mixed combat bonuses */
	INSCRIP_AVERAGE = 2,         /*!< Item with no interesting features */
	INSCRIP_MAGICAL = 3,         /*!< Item with combat bonuses */
	INSCRIP_SPLENDID = 4,        /*!< Obviously good item */
	INSCRIP_EXCELLENT = 5,       /*!< Ego-item */
	INSCRIP_SPECIAL = 6,         /*!< Artifact */
	INSCRIP_UNKNOWN = 7,

	INSCRIP_MAX                  /*!< Maximum number of pseudo-ID markers */
} obj_pseudo_t;

extern s32b object_last_wield;

bool easy_know(const struct object *obj);
bool object_all_but_flavor_is_known(const struct object *obj);
bool object_is_known_artifact(const struct object *obj);
bool object_is_known_not_artifact(const struct object *obj);
bool object_was_worn(const struct object *obj);
bool object_flavor_is_aware(const struct object *obj);
bool object_flavor_was_tried(const struct object *obj);
bool object_effect_is_known(const struct object *obj);
bool object_flag_is_known(const struct object *obj, int flag);
bool object_element_is_known(const struct object *obj, int element);
void object_set_base_known(struct object *obj);
void object_flavor_aware(struct object *obj);
void object_flavor_tried(struct object *obj);
bool object_notice_curses(struct object *obj);
void object_notice_on_use(struct object *obj);


#endif /* OBJECT_IDENTIFY_H */
