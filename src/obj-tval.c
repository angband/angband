/**
 * \file obj-tval.c
 * \brief Wrapper functions for tvals.
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

#include "init.h"
#include "obj-tval.h"
#include "z-type.h"
#include "z-util.h"

bool tval_is_staff(const struct object *obj)
{
	return obj->tval == TV_STAFF;
}

bool tval_is_wand(const struct object *obj)
{
	return obj->tval == TV_WAND;
}

bool tval_is_rod(const struct object *obj)
{
	return obj->tval == TV_ROD;
}

bool tval_is_potion(const struct object *obj)
{
	return obj->tval == TV_POTION;
}

bool tval_is_scroll(const struct object *obj)
{
	return obj->tval == TV_SCROLL;
}

bool tval_is_food(const struct object *obj)
{
	return obj->tval == TV_FOOD;
}

bool tval_is_food_k(const struct object_kind *kind)
{
	return kind->tval == TV_FOOD;
}

bool tval_is_mushroom(const struct object *obj)
{
	return obj->tval == TV_MUSHROOM;
}

bool tval_is_mushroom_k(const struct object_kind *kind)
{
	return kind->tval == TV_MUSHROOM;
}

bool tval_is_light(const struct object *obj)
{
	return obj->tval == TV_LIGHT;
}

bool tval_is_light_k(const struct object_kind *kind)
{
	return kind->tval == TV_LIGHT;
}

bool tval_is_ring(const struct object *obj)
{
	return obj->tval == TV_RING;
}

bool tval_is_chest(const struct object *obj)
{
	return obj->tval == TV_CHEST;
}

bool tval_is_fuel(const struct object *obj)
{
	return obj->tval == TV_FLASK;
}

bool tval_is_money(const struct object *obj)
{
	return obj->tval == TV_GOLD;
}

bool tval_is_money_k(const struct object_kind *kind)
{
	return kind->tval == TV_GOLD;
}

bool tval_is_pointy(const struct object *obj)
{
	return obj->tval == TV_SWORD || obj->tval == TV_POLEARM;
}

bool tval_is_digger(const struct object *obj)
{
	return obj->tval == TV_DIGGING;
}

bool tval_can_have_nourishment(const struct object *obj)
{
	return obj->tval == TV_FOOD || obj->tval == TV_POTION ||
			obj->tval == TV_MUSHROOM;
}

bool tval_can_have_charges(const struct object *obj)
{
	return obj->tval == TV_STAFF || obj->tval == TV_WAND;
}

bool tval_can_have_timeout(const struct object *obj)
{
	return obj->tval == TV_ROD;
}

bool tval_is_body_armor(const struct object *obj)
{
	switch (obj->tval) {
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
			return true;
		default:
			return false;
	}
}

bool tval_is_head_armor(const struct object *obj)
{
	return obj->tval == TV_HELM || obj->tval == TV_CROWN;
}

bool tval_is_ammo(const struct object *obj)
{
	switch (obj->tval) {
		case TV_SHOT:
		case TV_ARROW:
		case TV_BOLT:
			return true;
		default:
			return false;
	}
}

bool tval_is_launcher(const struct object *obj)
{
	return obj->tval == TV_BOW;
}

bool tval_is_useable(const struct object *obj)
{
	switch (obj->tval) {
		case TV_ROD:
		case TV_WAND:
		case TV_STAFF:
		case TV_SCROLL:
		case TV_POTION:
		case TV_FOOD:
		case TV_MUSHROOM:
			return true;
		default:
			return false;
	}
}

bool tval_can_have_failure(const struct object *obj)
{
	switch (obj->tval) {
		case TV_STAFF:
		case TV_WAND:
		case TV_ROD:
			return true;
		default:
			return false;
	}
}

bool tval_is_jewelry(const struct object *obj)
{
	return obj->tval == TV_RING || obj->tval == TV_AMULET;
}

bool tval_is_weapon(const struct object *obj)
{
	switch (obj->tval) {
		case TV_SWORD:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_DIGGING:
		case TV_BOW:
		case TV_BOLT:
		case TV_ARROW:
		case TV_SHOT:
			return true;
		default:
			return false;
	}
}

bool tval_is_armor(const struct object *obj)
{
	switch (obj->tval) {
		case TV_DRAG_ARMOR:
		case TV_HARD_ARMOR:
		case TV_SOFT_ARMOR:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_CROWN:
		case TV_HELM:
		case TV_BOOTS:
		case TV_GLOVES:
			return true;
		default:
			return false;
	}
}

bool tval_is_melee_weapon(const struct object *obj)
{
	switch (obj->tval) {
		case TV_SWORD:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_DIGGING:
			return true;
		default:
			return false;
	}
}

bool tval_has_variable_power(const struct object *obj)
{
	switch (obj->tval) {
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
			return true;
		default:
			return false;
	}
}

bool tval_is_wearable(const struct object *obj)
{
	switch (obj->tval) {
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
			return true;
		default:
			return false;
	}
}

bool tval_is_edible(const struct object *obj)
{
	switch (obj->tval) {
		case TV_FOOD:
		case TV_MUSHROOM:
			return true;
		default:
			return false;
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
		case TV_MUSHROOM:
		case TV_SCROLL:
			return true;
		default:
			return false;
	}
}

bool tval_is_book_k(const struct object_kind *kind)
{
	switch (kind->tval) {
		case TV_MAGIC_BOOK:
		case TV_PRAYER_BOOK:
		case TV_NATURE_BOOK:
		case TV_DEATH_BOOK:
		case TV_OTHER_BOOK:
			return true;
		default:
			return false;
	}
}

bool tval_is_zapper(const struct object *obj)
{
	return obj->tval == TV_WAND || obj->tval == TV_STAFF;
}

/**
 * List of { tval, name } pairs.
 */
static const grouper tval_names[] =
{
	#define TV(a, b, c) { TV_##a, b },
	#include "list-tvals.h"
	#undef TV
};

/**
 * Small hack to allow both spellings of armer
 */
char *de_armour(const char *name)
{
	char newname[40];
	char *armour;

	my_strcpy(newname, name, sizeof(newname));
	armour = strstr(newname, "armour");
	if (armour)
		my_strcpy(armour + 4, "r", 2);

	return string_make(newname);
}

/**
 * Returns the numeric equivalent tval of the textual tval `name`.
 */
int tval_find_idx(const char *name)
{
	size_t i = 0;
	unsigned int r;
	char *mod_name;

	if (sscanf(name, "%u", &r) == 1)
		return r;

	mod_name = de_armour(name);

	for (i = 0; i < N_ELEMENTS(tval_names); i++) {
		if (!my_stricmp(mod_name, tval_names[i].name)) {
			string_free(mod_name);
			return tval_names[i].tval;
		}
	}

	string_free(mod_name);
	return -1;
}

/**
 * Returns the textual equivalent tval of the numeric tval `name`.
 */
const char *tval_find_name(int tval)
{
	size_t i = 0;

	for (i = 0; i < N_ELEMENTS(tval_names); i++)
	{
		if (tval == tval_names[i].tval)
			return tval_names[i].name;
	}

	return "unknown";
}

/**
 * Counts the svals (from object.txt) of a given non-null tval
 */
int tval_sval_count(const char *name)
{
	size_t i, num = 0;
	int tval = tval_find_idx(name);

	if (tval < 0) return 0;

	for (i = 0; i < z_info->k_max; i++) {
		struct object_kind *kind = &k_info[i];

		if (!kind->tval) continue;
		if (kind->tval != tval) continue;
		num++;
	}

	return num;
}

/**
 * Lists up to max_size svals (from object.txt) of a given non-null tval
 * Assumes list has allocated space for at least max_size elements
 */
int tval_sval_list(const char *name, int *list, int max_size)
{
	size_t i;
	int num = 0;
	int tval = tval_find_idx(name);

	if (tval < 0) return 0;

	for (i = 0; i < z_info->k_max; i++) {
		struct object_kind *kind = &k_info[i];

		if (!kind->tval) continue;
		if (kind->tval != tval) continue;
		if (num >= max_size) break;
		list[num++] = kind->sval;
	}

	return num;
}
