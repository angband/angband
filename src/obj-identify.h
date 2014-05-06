/*
 * File: obj-identify.h
 * Purpose: Object identification and knowledge routines
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

/* ID flags */
#define IDENT_SENSE     0x0001  /* Has been "sensed" */
#define IDENT_WORN      0x0002  /* Has been tried on */
#define IDENT_KNOWN     0x0008  /* Fully known */
#define IDENT_STORE     0x0010  /* Item is in the inventory of a store */
#define IDENT_EFFECT    0x0080  /* Know item activation/effect */
/* xxx */
#define IDENT_FIRED     0x0800  /* Has been used as a missile */
#define IDENT_FAKE      0x2000  /* Item is a fake, for displaying knowledge */

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

bool easy_know(const object_type *o_ptr);
bool object_is_known(const object_type *o_ptr);
bool object_is_known_artifact(const object_type *o_ptr);
bool object_is_known_blessed(const object_type *o_ptr);
bool object_is_known_not_artifact(const object_type *o_ptr);
bool object_was_worn(const object_type *o_ptr);
bool object_was_fired(const object_type *o_ptr);
bool object_was_sensed(const object_type *o_ptr);
bool object_flavor_is_aware(const object_type *o_ptr);
bool object_flavor_was_tried(const object_type *o_ptr);
bool object_effect_is_known(const object_type *o_ptr);
bool object_name_is_visible(const object_type *o_ptr);
bool object_ego_is_visible(const object_type *o_ptr);
bool object_attack_plusses_are_visible(const object_type *o_ptr);
bool object_defence_plusses_are_visible(const object_type *o_ptr);
bool object_flag_is_known(const object_type *o_ptr, int flag);
bool object_element_is_known(const object_type *o_ptr, int element);
bool object_this_mod_is_visible(const object_type *o_ptr, int mod);
bool object_high_resist_is_possible(const object_type *o_ptr);
bool object_check_for_ident(object_type *o_ptr);
void object_flavor_aware(object_type *o_ptr);
void object_flavor_tried(object_type *o_ptr);
void object_know_all_flags(object_type *o_ptr);
void object_know_all_elements(object_type *o_ptr);
void object_know_brands_and_slays(object_type *o_ptr);
void object_notice_everything(object_type *o_ptr);
void object_notice_ego(object_type *o_ptr);
void object_notice_sensing(object_type *o_ptr);
void object_sense_artifact(object_type *o_ptr);
void object_notice_effect(object_type *o_ptr);
void object_notice_attack_plusses(object_type *o_ptr);
bool object_notice_element(object_type *o_ptr, int element);
bool object_notice_flag(object_type *o_ptr, int flag);
bool object_notice_flags(object_type *o_ptr, bitflag flags[OF_SIZE]);
bool object_notice_curses(object_type *o_ptr);
void object_notice_on_defend(struct player *p);
void object_notice_on_firing(object_type *o_ptr);
void object_notice_on_wield(object_type *o_ptr);
void wieldeds_notice_flag(struct player *p, int flag);
void wieldeds_notice_element(struct player *p, int element);
void wieldeds_notice_to_hit_on_attack(void);
void wieldeds_notice_on_attack(void);

/* Ostracism line */
obj_pseudo_t object_pseudo(const object_type *o_ptr);
void sense_inventory(void);

#endif /* OBJECT_IDENTIFY_H */
