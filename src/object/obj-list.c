/*
 * File: obj-list.c
 * Purpose: Object list UI.
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

#include "object.h"
#include "tvalsval.h"
#include "squelch.h"
#include "obj-list.h"

typedef struct object_list_entry_s {
	object_type *object;
	u16b count;
	s16b dx, dy;
} object_list_entry_t;

typedef struct object_list_s {
	object_list_entry_t *entries;
	size_t entries_size;
	s32b creation_turn;
	u16b total_entries;
	u16b total_objects;
	bool sorted;
} object_list_t;

/**
 * Allocate a new object list.
 */
static object_list_t *object_list_new(void)
{
	object_list_t *list = ZNEW(object_list_t);
	size_t size = MAX_ITEMLIST;

	if (list == NULL)
		return NULL;

	list->entries = C_ZNEW(size, object_list_entry_t);

	if (list->entries == NULL) {
		FREE(list);
		return NULL;
	}

	list->entries_size = size;

	return list;
}

/**
 * Free an object list.
 */
static void object_list_free(object_list_t *list)
{
	if (list == NULL)
		return;

	if (list->entries != NULL) {
		FREE(list->entries);
		list->entries = NULL;
	}

	FREE(list);
	list = NULL;
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
static object_list_t *object_list_shared_instance(void)
{
	if (object_list_subwindow == NULL) {
		object_list_subwindow = object_list_new();
	}

	return object_list_subwindow;
}

/**
 * Return TRUE if the list needs to be updated. Usually this is each turn.
 */
static bool object_list_needs_update(const object_list_t *list)
{
	if (list == NULL || list->entries == NULL)
		return FALSE;

	/* For now, always update when requested. */
	return TRUE;
}

/**
 * Zero out the contents of an object list.
 */
static void object_list_reset(object_list_t *list)
{
	if (list == NULL || list->entries == NULL)
		return;

	if (!object_list_needs_update(list))
		return;

	C_WIPE(list->entries, list->entries_size, object_list_entry_t);
	list->total_entries = 0;
	list->total_objects = 0;
	list->creation_turn = 0;
	list->sorted = FALSE;
}

/**
 * Return TRUE if the object should be omitted from the object list.
 */
static bool object_list_should_ignore_object(const object_type *object)
{
	if (object->kind == NULL)
		return TRUE;

	if (!object->marked)
		return TRUE;

	if (!is_unknown(object) && squelch_item_ok(object))
		return TRUE;

	if (object->tval == TV_GOLD)
		return TRUE;

	return FALSE;
}

/**
 * Collect object information from the current cave.
 */
static void object_list_collect(object_list_t *list)
{
	int item;
	int i;

	if (list == NULL || list->entries == NULL)
		return;

	if (!object_list_needs_update(list))
		return;

	/* Scan each object in the dungeon. */
	for (item = 0; item < z_info->o_max; item++) {
		object_type *object = object_byid(item);
		object_list_entry_t *entry = NULL;
		int entry_index;
		int current_distance;
		int entry_distance;

		if (object_list_should_ignore_object(object))
			continue;

		/* Find or add a list entry. */
		for (entry_index = 0; entry_index < (int)list->entries_size; entry_index++) {
			if (list->entries[entry_index].object == NULL) {
				/* We found an empty slot, so add this object here. */
				list->entries[entry_index].object = object;
				list->entries[entry_index].count = 0;
				list->entries[entry_index].dy = object->iy - p_ptr->py;
				list->entries[entry_index].dx = object->ix - p_ptr->px;
				entry = &list->entries[entry_index];
				break;
			}
			else if (!is_unknown(object) && object_similar(object, list->entries[entry_index].object, OSTACK_LIST)) {
				/* We found a matching object and we'll use that. */
				entry = &list->entries[entry_index];
				break;
			}
		}

		if (entry == NULL)
			return;

		/* We only know the number of objects if we've actually seen them. */
		if (object->marked == MARK_SEEN)
			entry->count += object->number;
		else
			entry->count = 1;

		/* Store the distance to the object in the stack that is closest to the player. */
		current_distance = (object->iy - p_ptr->py) * (object->iy - p_ptr->py) + (object->ix - p_ptr->px) * (object->ix - p_ptr->px);
		entry_distance = entry->dy * entry->dy + entry->dx * entry->dx;

		if (current_distance < entry_distance) {
			entry->dy = object->iy - p_ptr->py;
			entry->dx = object->ix - p_ptr->px;
		}
	}

	/* Collect totals for easier calculations of the list. */
	for (i = 0; i < (int)list->entries_size; i++) {
		if (list->entries[i].object == NULL)
			continue;

		if (list->entries[i].count > 0)
			list->total_entries++;

		list->total_objects += list->entries[i].count;
	}

	list->creation_turn = turn;
	list->sorted = FALSE;
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
static int object_list_standard_compare(const void *a, const void *b)
{
	int result;
	const object_type *ao = ((object_list_entry_t *)a)->object;
	const object_type *bo = ((object_list_entry_t *)b)->object;

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
static void object_list_sort(object_list_t *list, int (*compare)(const void *, const void *))
{
	size_t elements;

	if (list == NULL || list->entries == NULL)
		return;

	if (list->sorted)
		return;

	elements = list->total_entries;

	if (elements <= 1)
		return;

	sort(list->entries, elements, sizeof(list->entries[0]), compare);
	list->sorted = TRUE;
}

/**
 * Return an attribute to display a particular list entry with.
 *
 * \param entry is the object list entry to display.
 * \return a term attribute for the object entry.
 */
static byte object_list_entry_line_attribute(const object_list_entry_t *entry)
{
	byte attr;

	if (entry == NULL || entry->object == NULL || entry->object->kind == NULL)
		return TERM_WHITE;

	if (is_unknown(entry->object))
	/* unknown object */
		attr = TERM_RED;
	else if (entry->object->artifact && object_is_known(entry->object))
	/* known artifact */
		attr = TERM_VIOLET;
	else if (!object_flavor_is_aware(entry->object))
	/* unaware of kind */
		attr = TERM_L_RED;
	else if (entry->object->kind->cost == 0)
	/* worthless */
		attr = TERM_SLATE;
	else
	/* default */
		attr = TERM_WHITE;

	return attr;
}

/**
 * Format the object name so that the prefix is right aligned to a common column.
 *
 * This uses the default logic of object_desc() in order to handle flavors, artifacts,
 * vowels and so on. It was easier to do this and then use strtok() to break it up than
 * to do anything else.
 *
 * \param entry is the object list entry that has a name to be formatted.
 * \param line_buffer is the buffer to format into.
 * \param size is the size of line_buffer.
 * \param full_width is the maximum formatted width allowed.
 */
static void object_list_format_name(const object_list_entry_t *entry, char *line_buffer, size_t size, size_t full_width)
{
	char name[80];
	const char *chunk;
	char *source;
	size_t name_width = MIN(full_width, size);
	bool has_singular_prefix;
	byte old_number;

	if (entry == NULL || entry->object == NULL || entry->object->kind == NULL)
		return;

	/* Hack - these don't have a prefix when there is only one, so just pad with a space. */
	switch (entry->object->kind->tval) {
		case TV_SOFT_ARMOR:
			has_singular_prefix = (entry->object->kind->sval == SV_ROBE);
			break;
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
			if ((object_name_is_visible(entry->object) || object_is_known(entry->object)) && entry->object->artifact)
				has_singular_prefix = TRUE;
			else
				has_singular_prefix = FALSE;				
			break;
		default:
			has_singular_prefix = TRUE;
			break;
	}

	if (entry->object->marked == MARK_AWARE)
		has_singular_prefix = TRUE;

	/*
	 * Because each entry points to a specific object and not something more general, the
	 * number of similar objects we counted has to be swapped in. This isn't an ideal way
	 * to do this, but it's the easiest way until object_desc is more flexible.
	 */
	old_number = entry->object->number;
	entry->object->number = entry->count;
	object_desc(name, sizeof(name), entry->object, ODESC_PREFIX | ODESC_FULL);
	entry->object->number = old_number;

	/* The source string for strtok() needs to be set properly, depending on when we use it. */
	if (!has_singular_prefix && entry->count == 1) {
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
	my_strcat(line_buffer, chunk, name_width + 1);
}

/**
 * Format a section of the object list: a header followed by object list entry rows.
 *
 * This function will process each entry for the given section. It will display:
 * - object char;
 * - number of objects;
 * - object name (truncated, if needed to fit the line);
 * - object distance from the player (aligned to the right side of the list).
 * By passing in a NULL textblock, the maximum line width of the section can be found.
 *
 * \param list is the object list to format.
 * \param tb is the textblock to produce or NULL if only the dimensions need to be calculated.
 * \param lines_to_display are the number of entries to display (not including the header).
 * \param max_width is the maximum line width.
 * \param prefix is the beginning of the header; the remainder is appended with the number of objects.
 * \param max_width_result is returned with the width needed to format the list without truncation.
 */
static void object_list_format_section(const object_list_t *list, textblock *tb, int lines_to_display, int max_width, const char *prefix, size_t *max_width_result)
{
	int remaining_object_total = 0;
	int line_count = 0;
	int entry_index;
	int total;
	char line_buffer[200];
	const char *punctuation = (lines_to_display == 0) ? "." : ":";
	size_t max_line_length = 0;

	if (list == NULL || list->entries == NULL)
		return;

	total = list->total_entries;

	if (list->total_entries == 0) {
		max_line_length = strnfmt(line_buffer, sizeof(line_buffer), "%s no objects.\n", prefix);

		if (tb != NULL)
			textblock_append(tb, "%s", line_buffer);

		/* Force a minimum width so that the prompt doesn't get cut off. */
		if (max_width_result != NULL)
			*max_width_result = MAX(max_line_length, 40);

		return;
	}

	max_line_length = strnfmt(line_buffer, sizeof(line_buffer), "%s %d object%s%s\n", prefix, list->total_entries, PLURAL(list->total_entries), punctuation);

	if (tb != NULL)
		textblock_append(tb, "%s", line_buffer);

	for (entry_index = 0; entry_index < total && line_count < lines_to_display; entry_index++) {
		char location[20] = { '\0' };
		byte line_attr;
		size_t full_width;
		const char *direction_y = (list->entries[entry_index].dy <= 0) ? "N" : "S";
		const char *direction_x = (list->entries[entry_index].dx <= 0) ? "W" : "E";

		line_buffer[0] = '\0';

		if (list->entries[entry_index].count == 0)
			continue;

		/* Build the location string. */
		strnfmt(location, sizeof(location), " %d %s %d %s", abs(list->entries[entry_index].dy), direction_y, abs(list->entries[entry_index].dx), direction_x);

		/* Get width available for object name: 2 for char and space; location includes padding; last -1 for some reason? */
		full_width = max_width - 2 - strlen(location) - 1;

		/* Add the object count and clip the object name to fit. */
		object_list_format_name(&list->entries[entry_index], line_buffer, sizeof(line_buffer), full_width);

		/* Calculate the width of the line for dynamic sizing; use a fixed max width for location and object char. */
		max_line_length = MAX(max_line_length, strlen(line_buffer) + 12 + 2);

		/* textblock_append_pict will safely add the object symbol, regardless of ASCII/graphics mode. */
		if (tb != NULL && tile_width == 1 && tile_height == 1) {
			byte a = TERM_RED;
			wchar_t c = L'*';

			if (!is_unknown(list->entries[entry_index].object) && list->entries[entry_index].object->kind != NULL) {
				a = object_kind_attr(list->entries[entry_index].object->kind);
				c = object_kind_char(list->entries[entry_index].object->kind);
			}

			textblock_append_pict(tb, a, c);
			textblock_append(tb, " ");
		}

		/* Add the left-aligned and padded object name which will align the location to the right. */
		if (tb != NULL) {
			/*
			 * Hack - Because object name strings are UTF8, we have to add additional padding for
			 * any raw bytes that might be consolidated into one displayed character.
			 */
			full_width += strlen(line_buffer) - Term_mbstowcs(NULL, line_buffer, 0);
			line_attr = object_list_entry_line_attribute(&list->entries[entry_index]);
			textblock_append_c(tb, line_attr, "%-*s%s\n", full_width, line_buffer, location);
		}

		line_count++;
	}

	/* Don't worry about the "...others" line, since it's probably shorter than what's already printed. */
	if (max_width_result != NULL)
		*max_width_result = max_line_length;

	/* Bail since we don't have enough room to display the remaining count or since we've displayed them all. */
	if (lines_to_display <= 0 || lines_to_display >= list->total_entries)
		return;

	/* Count the remaining objects, starting where we left off in the above loop. */
	remaining_object_total = total - entry_index;

	if (tb != NULL)
		textblock_append(tb, "%6s...and %d others.\n", " ", remaining_object_total);
}

/**
 * Allow the standard list formatted to be bypassed for special cases.
 *
 * Returning TRUE will bypass any other formatteding in object_list_format_textblock().
 *
 * \param list is the object list to format.
 * \param tb is the textblock to produce or NULL if only the dimensions need to be calculated.
 * \param max_lines is the maximum number of lines that can be displayed.
 * \param max_width is the maximum line width that can be displayed.
 * \param max_height_result is returned with the number of lines needed to format the list without truncation.
 * \param max_width_result is returned with the width needed to format the list without truncation.
 * \return TRUE if further formatting should be bypassed.
 */
static bool object_list_format_special(const object_list_t *list, textblock *tb, int max_lines, int max_width, size_t *max_height_result, size_t *max_width_result)
{
	return FALSE;
}

/**
 * Format the entire object list with the given parameters.
 *
 * This function can be used to calculate the preferred dimensions for the list by passing in a
 * NULL textblock. This function calls object_list_format_special() first; if that function
 * returns TRUE, it will bypass normal list formatting.
 *
 * \param list is the object list to format.
 * \param tb is the textblock to produce or NULL if only the dimensions need to be calculated.
 * \param max_lines is the maximum number of lines that can be displayed.
 * \param max_width is the maximum line width that can be displayed.
 * \param max_height_result is returned with the number of lines needed to format the list without truncation.
 * \param max_width_result is returned with the width needed to format the list without truncation.
 */
static void object_list_format_textblock(const object_list_t *list, textblock *tb, int max_lines, int max_width, size_t *max_height_result, size_t *max_width_result)
{
	int header_lines = 1;
	int lines_remaining;
	int lines_to_display;
	size_t max_line_width = 0;

	if (list == NULL || list->entries == NULL)
		return;

	if (object_list_format_special(list, tb, max_lines, max_width, max_height_result, max_width_result))
		return;

	lines_to_display = list->total_entries;

	if (max_height_result != NULL)
		*max_height_result = header_lines + lines_to_display;

	lines_remaining = max_lines - header_lines;

	if (lines_remaining < list->total_entries)
		lines_to_display = MAX(lines_remaining - 1, 0);

	if (header_lines >= max_lines)
		lines_to_display = 0;

	object_list_format_section(list, tb, lines_to_display, max_width, "You can see", &max_line_width);

	if (max_width_result != NULL)
		*max_width_result = max_line_width;
}

/**
 * Display the object list statically. This will force the list to be displayed to
 * the provided dimensions. Contents will be adjusted accordingly.
 *
 * In order to be more efficient, this function uses a shared list object so that
 * it's not constantly allocating and freeing the list.
 *
 * \param height is the height of the list.
 * \param width is the width of the list.
 */
void object_list_show_subwindow(int height, int width)
{
	textblock *tb;
	object_list_t *list;

	if (height < 1 || width < 1)
		return;

	tb = textblock_new();
	list = object_list_shared_instance();

	object_list_reset(list);
	object_list_collect(list);
	object_list_sort(list, object_list_standard_compare);

	/* Draw the list to exactly fit the subwindow. */
	object_list_format_textblock(list, tb, height, width, NULL, NULL);
	textui_textblock_place(tb, SCREEN_REGION, NULL);

	textblock_free(tb);
}

/**
 * Display the object list interactively. This will dynamically size the list for
 * the best appearance. This should only be used in the main term.
 *
 * \param height is the height limit for the list.
 * \param width is the width limit for the list.
 */
void object_list_show_interactive(int height, int width)
{
	textblock *tb;
	object_list_t *list;
	size_t max_width = 0, max_height = 0;
	int safe_height, safe_width;
	region r;

	if (height < 1 || width < 1)
		return;

	tb = textblock_new();
	list = object_list_new();

	object_list_collect(list);
	object_list_sort(list, object_list_standard_compare);

	/*
	 * Figure out optimal display rect. Large numbers are passed as the height and
	 * width limit so that we can calculate the maximum number of rows and columns
	 * to display the list nicely. We then adjust those values as needed to fit in
	 * the main term. Height is adjusted to account for the texblock prompt. The
	 * list is positioned on the right side of the term underneath the status line.
	 */
	object_list_format_textblock(list, NULL, 1000, 1000, &max_height, &max_width);
	safe_height = MIN(height - 2, (int)max_height + 2);
	safe_width = MIN(width - 13, (int)max_width);
	r.col = -safe_width;
	r.row = 1;
	r.width = safe_width;
	r.page_rows = safe_height;

	/*
	 * Actually draw the list. We pass in max_height to the format function so that
	 * all lines will be appended to the textblock. The textblock itself will handle
	 * fitting it into the region. However, we have to pass safe_width so that the
	 * format function will pad the lines properly so that the location string is
	 * aligned to the right edge of the list.
	 */
	object_list_format_textblock(list, tb, (int)max_height, safe_width, NULL, NULL);
	region_erase_bordered(&r);
	textui_textblock_show(tb, r, NULL);

	textblock_free(tb);
	object_list_free(list);
}
