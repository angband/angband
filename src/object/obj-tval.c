/*
 * File: obj-tval.c
 * Purpose: Wrapper functions for tvals.
 *
 * Copyright (c) 2014 Ben Semmler
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

#include "obj-tval.h"

#include "tvalsval.h"

bool tval_is_staff(const struct object *o_ptr)
{
	return o_ptr->tval == TV_STAFF;
}

bool tval_is_wand(const struct object *o_ptr)
{
	return o_ptr->tval == TV_WAND;
}

bool tval_is_rod(const struct object *o_ptr)
{
	return o_ptr->tval == TV_ROD;
}

bool tval_is_potion(const struct object *o_ptr)
{
	return o_ptr->tval == TV_POTION;
}

bool tval_is_scroll(const struct object *o_ptr)
{
	return o_ptr->tval == TV_SCROLL;
}

bool tval_is_food(const struct object *o_ptr)
{
	return o_ptr->tval == TV_FOOD;
}

bool tval_is_food_k(const struct object_kind *kind)
{
	return kind->tval == TV_FOOD;
}

bool tval_is_light(const struct object *o_ptr)
{
	return o_ptr->tval == TV_LIGHT;
}

bool tval_is_light_k(const struct object_kind *kind)
{
	return kind->tval == TV_LIGHT;
}

bool tval_is_ring(const struct object *o_ptr)
{
	return o_ptr->tval == TV_RING;
}

bool tval_is_chest(const struct object *o_ptr)
{
	return o_ptr->tval == TV_CHEST;
}

bool tval_is_fuel(const struct object *o_ptr)
{
	return o_ptr->tval == TV_FLASK;
}

bool tval_is_money(const struct object *o_ptr)
{
	return o_ptr->tval == TV_GOLD;
}

bool tval_is_money_k(const struct object_kind *kind)
{
	return kind->tval == TV_GOLD;
}

bool tval_is_pointy(const struct object *o_ptr)
{
	return o_ptr->tval == TV_SWORD || o_ptr->tval == TV_POLEARM;
}

bool tval_can_have_nourishment(const struct object *o_ptr)
{
	return o_ptr->tval == TV_FOOD || o_ptr->tval == TV_POTION;
}

bool tval_can_have_charges(const struct object *o_ptr)
{
	return o_ptr->tval == TV_STAFF || o_ptr->tval == TV_WAND;
}

bool tval_can_have_timeout(const struct object *o_ptr)
{
	return o_ptr->tval == TV_ROD;
}

bool tval_is_body_armor(const struct object *o_ptr)
{
	switch (o_ptr->tval) {
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
			return TRUE;
		default:
			return FALSE;
	}
}

bool tval_is_head_armor(const struct object *o_ptr)
{
	return o_ptr->tval == TV_HELM || o_ptr->tval == TV_CROWN;
}

bool tval_is_ammo(const struct object *o_ptr)
{
	switch (o_ptr->tval) {
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
			return TRUE;
		default:
			return FALSE;
	}
}

bool tval_is_useable(const struct object *o_ptr)
{
	switch (o_ptr->tval) {
		case TV_ROD:
		case TV_WAND:
		case TV_STAFF:
		case TV_SCROLL:
		case TV_POTION:
		case TV_FOOD:
			return TRUE;
		default:
			return FALSE;
	}
}

bool tval_can_have_failure(const struct object *o_ptr)
{
	switch (o_ptr->tval) {
		case TV_STAFF:
		case TV_WAND:
		case TV_ROD:
			return TRUE;
		default:
			return FALSE;
	}
}

bool tval_is_jewelry(const struct object *o_ptr)
{
	return o_ptr->tval == TV_RING || o_ptr->tval == TV_AMULET;
}

bool tval_is_weapon(const struct object *o_ptr)
{
	switch (o_ptr->tval) {
		case TV_SWORD:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_DIGGING:
		case TV_BOW:
		case TV_BOLT:
		case TV_ARROW:
		case TV_SHOT:
			return TRUE;
		default:
			return FALSE;
	}
}

bool tval_is_armor(const struct object *o_ptr)
{
	switch (o_ptr->tval) {
		case TV_DRAG_ARMOR:
		case TV_HARD_ARMOR:
		case TV_SOFT_ARMOR:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_CROWN:
		case TV_HELM:
		case TV_BOOTS:
		case TV_GLOVES:
			return TRUE;
		default:
			return FALSE;
	}
}

bool tval_is_melee_weapon(const struct object *o_ptr)
{
	switch (o_ptr->tval) {
		case TV_SWORD:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_DIGGING:
			return TRUE;
		default:
			return FALSE;
	}
}

bool tval_is_wearable(const struct object *o_ptr)
{
	switch (o_ptr->tval) {
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
		case TV_BOW:
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		case TV_LIGHT:
		case TV_AMULET:
		case TV_RING:
			return TRUE;
		default:
			return FALSE;
	}
}

bool tval_can_have_flavor_k(const struct object_kind *kind)
{
	switch (kind->tval) {
		case TV_AMULET:
		case TV_RING:
		case TV_STAFF:
		case TV_WAND:
		case TV_ROD:
		case TV_POTION:
		case TV_FOOD:
		case TV_SCROLL:
			return TRUE;
		default:
			return FALSE;
	}
}

bool tval_is_book_k(const struct object_kind *kind)
{
	switch (kind->tval) {
		case TV_MAGIC_BOOK:
		case TV_PRAYER_BOOK:
			return TRUE;
		default:
			return FALSE;
	}
}

bool tval_is_zapper(const struct object *o_ptr)
{
	return o_ptr->tval == TV_WAND || o_ptr->tval == TV_STAFF;
}
