/**
 * \file obj-list.c
 * \brief Object list construction.
 *
 * Copyright (c) 1997-2007 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * Copyright (c) 2013 Ben Semmler
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
#include "game-world.h"
#include "obj-desc.h"
#include "obj-identify.h"
#include "obj-ignore.h"
#include "obj-list.h"
#include "obj-pile.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "project.h"

/**
 * Allocate a new object list.
 */
object_list_t *object_list_new(void)
{
	object_list_t *list = mem_zalloc(sizeof(object_list_t));
	size_t size = MAX_ITEMLIST;

	if (list == NULL)
		return NULL;

	list->entries = mem_zalloc(size * sizeof(object_list_entry_t));

	if (list->entries == NULL) {
		mem_free(list);
		return NULL;
	}

	list->entries_size = size;

	return list;
}

/**
 * Free an object list.
 */
void object_list_free(object_list_t *list)
{
	if (list == NULL)
		return;

	if (list->entries != NULL) {
		mem_free(list->entries);
		list->entries = NULL;
	}

	mem_free(list);
}

/**
 * Share object list instance.
 */
static object_list_t *object_list_subwindow = NULL;

/**
 * Initialize the object list module.
 */
void object_list_init(void)
{
	object_list_subwindow = NULL;
}

/**
 * Tear down the object list module.
 */
void object_list_finalize(void)
{
	object_list_free(object_list_subwindow);
}

/**
 * Return a common object list instance.
 */
object_list_t *object_list_shared_instance(void)
{
	if (object_list_subwindow == NULL) {
		object_list_subwindow = object_list_new();
	}

	return object_list_subwindow;
}

/**
 * Return true if the list needs to be updated. Usually this is each turn.
 */
static bool object_list_needs_update(const object_list_t *list)
{
	if (list == NULL || list->entries == NULL)
		return false;

	/* For now, always update when requested. */
	return true;
}

/**
 * Zero out the contents of an object list.
 */
void object_list_reset(object_list_t *list)
{
	if (list == NULL || list->entries == NULL)
		return;

	if (!object_list_needs_update(list))
		return;

	memset(list->entries, 0, list->entries_size * sizeof(object_list_entry_t));
	memset(list->total_entries, 0, OBJECT_LIST_SECTION_MAX * sizeof(u16b));
	memset(list->total_objects, 0, OBJECT_LIST_SECTION_MAX * sizeof(u16b));
	list->distinct_entries = 0;
	list->creation_turn = 0;
	list->sorted = false;
}

/**
 * Return true if the object should be omitted from the object list.
 */
static bool object_list_should_ignore_object(const struct object *obj)
{
	struct object *base_obj = cave->objects[obj->oidx];

	assert(obj->kind);
	assert(base_obj);

	if (!is_unknown(base_obj) && ignore_known_item_ok(obj))
		return true;

	if (tval_is_money(base_obj))
		return true;

	return false;
}

/**
 * Collect object information from the current cave.
 */
void object_list_collect(object_list_t *list)
{
	int i;
	int py = player->py;
	int px = player->px;

	if (list == NULL || list->entries == NULL)
		return;

	if (!object_list_needs_update(list))
		return;

	/* Scan each object in the dungeon. */
	for (i = 1; i < cave_k->obj_max; i++) {
		object_list_entry_t *entry = NULL;
		int entry_index;
		int current_distance;
		int entry_distance;
		int y, x, field;
		bool los = false;
		struct object *obj = cave_k->objects[i];

		/* Skip unfilled entries, unknown objects and monster-held objects */
		if (!obj) continue;
		if ((obj->iy == 0) && (obj->ix == 0)) {
			continue;
		} else {
			y = obj->iy;
			x = obj->ix;
		}

		/* Determine which section of the list the object entry is in */
		los = projectable(cave, py, px, y, x, PROJECT_NONE) ||
			((y == py) && (x == px));
		field = (los) ? OBJECT_LIST_SECTION_LOS : OBJECT_LIST_SECTION_NO_LOS;

		if (object_list_should_ignore_object(obj)) continue;

		/* Find or add a list entry. */
		for (entry_index = 0; entry_index < (int)list->entries_size;
			 entry_index++) {
			int j;
			struct object *list_obj = list->entries[entry_index].object;

			if (list_obj == NULL) {
				/* We found an empty slot, so add this object here. */
				list->entries[entry_index].object = obj;
				for (j = 0; j < OBJECT_LIST_SECTION_MAX; j++)
					list->entries[entry_index].count[j] = 0;
				list->entries[entry_index].dy = y - player->py;
				list->entries[entry_index].dx = x - player->px;
				entry = &list->entries[entry_index];
				break;
			} else if (!is_unknown(obj)) {
				/* Use a matching object if we find one. */
				struct object *obj1 = cave->objects[obj->oidx];
				struct object *obj2 = cave->objects[list_obj->oidx];
 
				if (object_similar(obj1, obj2, OSTACK_LIST)) {
					entry = &list->entries[entry_index];
					break;
				}
			}
		}

		if (entry == NULL)
			return;

		/* We only know the number of objects we've actually seen */
		if (obj->kind == cave->objects[obj->oidx]->kind)
			entry->count[field] += obj->number;
		else
			entry->count[field] = 1;

		/* Store the distance to the object in the stack that is
		 * closest to the player. */
		current_distance = (y - player->py) * (y - player->py) +
			(x - player->px) * (x - player->px);
		entry_distance = entry->dy * entry->dy + entry->dx * entry->dx;

		if (current_distance < entry_distance) {
			entry->dy = y - player->py;
			entry->dx = x - player->px;
		}
	}

	/* Collect totals for easier calculations of the list. */
	for (i = 0; i < (int)list->entries_size; i++) {
		if (list->entries[i].object == NULL)
			continue;

		if (list->entries[i].count[OBJECT_LIST_SECTION_LOS] > 0)
			list->total_entries[OBJECT_LIST_SECTION_LOS]++;

		if (list->entries[i].count[OBJECT_LIST_SECTION_NO_LOS] > 0)
			list->total_entries[OBJECT_LIST_SECTION_NO_LOS]++;

		list->total_objects[OBJECT_LIST_SECTION_LOS] +=
			list->entries[i].count[OBJECT_LIST_SECTION_LOS];
		list->total_objects[OBJECT_LIST_SECTION_NO_LOS] +=
			list->entries[i].count[OBJECT_LIST_SECTION_NO_LOS];
		list->distinct_entries++;
	}

	list->creation_turn = turn;
	list->sorted = false;
}

/**
 * Object distance comparator: nearest to farthest.
 */
static int object_list_distance_compare(const void *a, const void *b)
{
	const object_list_entry_t *ae = (object_list_entry_t *)a;
	const object_list_entry_t *be = (object_list_entry_t *)b;
	int a_distance = ae->dy * ae->dy + ae->dx * ae->dx;
	int b_distance = be->dy * be->dy + be->dx * be->dx;

	if (a_distance < b_distance)
		return -1;
	else if (a_distance > b_distance)
		return 1;

	return 0;
}

/**
 * Standard comparison function for the object list. Uses compare_items().
 */
int object_list_standard_compare(const void *a, const void *b)
{
	int result;
	const struct object *ao = cave->objects[(((object_list_entry_t *)a)->object)->oidx];
	const struct object *bo = cave->objects[(((object_list_entry_t *)b)->object)->oidx];

	/* If this happens, something might be wrong in the collect function. */
	if (ao == NULL || bo == NULL)
		return 1;

	result = compare_items(ao, bo);

	/* If the objects are equivalent, sort nearest to farthest. */
	if (result == 0)
		result = object_list_distance_compare(a, b);

	return result;
}

/**
 * Sort the object list with the given sort function.
 */
void object_list_sort(object_list_t *list,
					  int (*compare)(const void *, const void *))
{
	size_t elements;

	if (list == NULL || list->entries == NULL)
		return;

	if (list->sorted)
		return;

	elements = list->distinct_entries;

	if (elements <= 1)
		return;

	sort(list->entries, elements, sizeof(list->entries[0]), compare);
	list->sorted = true;
}

/**
 * Return an attribute to display a particular list entry with.
 *
 * \param entry is the object list entry to display.
 * \return a term attribute for the object entry.
 */
byte object_list_entry_line_attribute(const object_list_entry_t *entry)
{
	byte attr;
	struct object *base_obj;

	if (entry == NULL || entry->object == NULL || entry->object->kind == NULL)
		return COLOUR_WHITE;

	base_obj = cave->objects[entry->object->oidx];

	if (is_unknown(base_obj))
		/* unknown object */
		attr = COLOUR_RED;
	else if (base_obj->known->artifact)
		/* known artifact */
		attr = COLOUR_VIOLET;
	else if (!object_flavor_is_aware(base_obj))
		/* unaware of kind */
		attr = COLOUR_L_RED;
	else if (base_obj->kind->cost == 0)
		/* worthless */
		attr = COLOUR_SLATE;
	else
		/* default */
		attr = COLOUR_WHITE;

	return attr;
}

/**
 * Format the object name so that the prefix is right aligned to a common
 * column.
 *
 * This uses the default logic of object_desc() in order to handle flavors,
 * artifacts, vowels and so on. It was easier to do this and then use strtok()
 * to break it up than to do anything else.
 *
 * \param entry is the object list entry that has a name to be formatted.
 * \param line_buffer is the buffer to format into.
 * \param size is the size of line_buffer.
 */
void object_list_format_name(const object_list_entry_t *entry,
							 char *line_buffer, size_t size)
{
	char name[80];
	const char *chunk;
	char *source;
	bool has_singular_prefix;
	bool los = false;
	int field;
	byte old_number;
	int py = player->py;
	int px = player->px;
	struct object *base_obj;
	int iy;
	int ix;

	if (entry == NULL || entry->object == NULL || entry->object->kind == NULL)
		return;

	base_obj = cave->objects[entry->object->oidx];
	iy = entry->object->iy;
	ix = entry->object->ix;

	/* Hack - these don't have a prefix when there is only one, so just pad
	 * with a space. */
	switch (entry->object->kind->tval) {
		case TV_SOFT_ARMOR:
			has_singular_prefix = (entry->object->kind->sval == lookup_sval(TV_SOFT_ARMOR, "Robe"));
			break;
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
			if (base_obj->known->artifact && entry->object->artifact)
				has_singular_prefix = true;
			else
				has_singular_prefix = false;				
			break;
		default:
			has_singular_prefix = true;
			break;
	}

	if (entry->object->kind != base_obj->kind)
		has_singular_prefix = true;

	/* Work out if the object is in view */
	los = projectable(cave, py, px, iy, ix, PROJECT_NONE) || 
		((iy == py) && (ix == px));
	field = los ? OBJECT_LIST_SECTION_LOS : OBJECT_LIST_SECTION_NO_LOS;

	/*
	 * Because each entry points to a specific object and not something more
	 * general, the number of similar objects we counted has to be swapped in.
	 * This isn't an ideal way to do this, but it's the easiest way until
	 * object_desc is more flexible.
	 */
	old_number = entry->object->number;
	entry->object->number = entry->count[field];
	object_desc(name, sizeof(name), base_obj, ODESC_PREFIX | ODESC_FULL);
	entry->object->number = old_number;

	/* The source string for strtok() needs to be set properly, depending on
	 * when we use it. */
	if (!has_singular_prefix && entry->count[field] == 1) {
		chunk = " ";
		source = name;
	}
	else {
		chunk = strtok(name, " ");
		source = NULL;
	}

	/* Right alight the prefix and clip. */
	strnfmt(line_buffer, size, "%3.3s ", chunk);

	/* Get the rest of the name and clip it to fit the max width. */
	chunk = strtok(source, "\0");
	my_strcat(line_buffer, chunk, size);
}

