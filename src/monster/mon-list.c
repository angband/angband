/*
 * File: mon-list.c
 * Purpose: Monster list UI.
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


#include "mon-util.h"
#include "mon-list.h"

typedef enum monster_list_section_e {
	MONSTER_LIST_SECTION_LOS = 0,
	MONSTER_LIST_SECTION_ESP,
	MONSTER_LIST_SECTION_MAX,
} monster_list_section_t;

typedef struct monster_list_entry_s {
	monster_race *race;
	u16b count[MONSTER_LIST_SECTION_MAX];
	u16b asleep[MONSTER_LIST_SECTION_MAX];
	s16b dx, dy;
	byte attr;
} monster_list_entry_t;

typedef struct monster_list_s {
	monster_list_entry_t *entries;
	size_t entries_size;
	u16b distinct_entries;
	s32b creation_turn;
	bool sorted;
	u16b total_entries[MONSTER_LIST_SECTION_MAX];
	u16b total_monsters[MONSTER_LIST_SECTION_MAX];
} monster_list_t;

/**
 * Allocate a new monster list based on the size of the current cave's monster array.
 */
static monster_list_t *monster_list_new(void)
{
	monster_list_t *list = ZNEW(monster_list_t);
	size_t size = cave_monster_max(cave);

	if (list == NULL)
		return NULL;

	list->entries = C_ZNEW(size, monster_list_entry_t);

	if (list->entries == NULL) {
		FREE(list);
		return NULL;
	}

	list->entries_size = size;

	return list;
}

/**
 * Free a monster list.
 */
static void monster_list_free(monster_list_t *list)
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
 * Shared monster list instance.
 */
static monster_list_t *monster_list_subwindow = NULL;

/**
 * Initialize the monster list module.
 */
void monster_list_init(void)
{
	monster_list_subwindow = NULL;
}

/**
 * Tear down the monster list module.
 */
void monster_list_finalize(void)
{
	monster_list_free(monster_list_subwindow);
}

/**
 * Return a common monster list instance.
 */
static monster_list_t *monster_list_shared_instance(void)
{
	if (monster_list_subwindow == NULL) {
		monster_list_subwindow = monster_list_new();
	}

	return monster_list_subwindow;
}

/**
 * Return TRUE if the list needs to be updated. Usually this is each turn or if
 * the number of cave monsters changes.
 */
static bool monster_list_needs_update(const monster_list_t *list)
{
	if (list == NULL || list->entries == NULL)
		return FALSE;

	return list->creation_turn != turn || (int)list->entries_size < cave_monster_max(cave);
}

/**
 * Zero out the contents of a monster list. If needed, this function will reallocate
 * the entry list if the number of monsters has changed.
 */
static void monster_list_reset(monster_list_t *list)
{
	if (list == NULL || list->entries == NULL)
		return;

	if (!monster_list_needs_update(list))
		return;

	if ((int)list->entries_size < cave_monster_max(cave)) {
		list->entries = mem_realloc(list->entries, sizeof(list->entries[0]) * cave_monster_max(cave));
		list->entries_size = cave_monster_max(cave);
	}

	C_WIPE(list->entries, list->entries_size, monster_list_entry_t);
	C_WIPE(&list->total_entries, MONSTER_LIST_SECTION_MAX, u16b);
	C_WIPE(&list->total_monsters, MONSTER_LIST_SECTION_MAX, u16b);
	list->distinct_entries = 0;
	list->creation_turn = 0;
	list->sorted = FALSE;
}

/**
 * Collect monster information from the current cave's monster list.
 */
static void monster_list_collect(monster_list_t *list)
{
	int i;

	if (list == NULL || list->entries == NULL)
		return;

	/* Use cave_monster_max() here in case the monster list isn't compacted. */
	for (i = 1; i < cave_monster_max(cave); i++) {
		monster_type *monster = cave_monster(cave, i);
		monster_list_entry_t *entry = NULL;
		int j, field;
		bool los = FALSE;

		/* Only consider visible, known monsters */
		if (!monster->ml || monster->unaware)
			continue;

		/* Find or add a list entry. */
		for (j = 0; j < (int)list->entries_size; j++) {
			if (list->entries[j].race == NULL) {
				/* We found an empty slot, so add this race here. */
				list->entries[j].race = monster->race;
				entry = &list->entries[j];
				break;
			}
			else if (list->entries[j].race == monster->race) {
				/* We found a matching race and we'll use that. */
				entry = &list->entries[j];
				break;
			}
		}

		if (entry == NULL)
			continue;

		/* Always collect the latest monster attribute so that flicker animation works. */
		entry->attr = (monster->attr > 0) ? monster->attr : monster->race->x_attr;

		/* Skip the projection and location checks if nothing has changed. */
		if (!monster_list_needs_update(list))
			continue;

		/*
		 * Check for LOS
		 * Hack - we should use (m_ptr->mflag & (MFLAG_VIEW)) here,
		 * but this does not catch monsters detected by ESP which are
		 * targetable, so we cheat and use projectable() instead
		 */
		los = projectable(p_ptr->py, p_ptr->px, monster->fy, monster->fx, PROJECT_NONE);
		field = (los) ? MONSTER_LIST_SECTION_LOS : MONSTER_LIST_SECTION_ESP;
		entry->count[field]++;

		if (monster->m_timed[MON_TMD_SLEEP] > 0)
			entry->asleep[field]++;

		/* Store the location offset from the player; this is only used for monster counts of 1 */
		entry->dx = monster->fx - p_ptr->px;
		entry->dy = monster->fy - p_ptr->py;
	}

	/* Skip calculations if nothing has changed, otherwise this will yield incorrect numbers. */
	if (!monster_list_needs_update(list))
		return;

	/* Collect totals for easier calculations of the list. */
	for (i = 0; i < (int)list->entries_size; i++) {
		if (list->entries[i].race == NULL)
			continue;

		if (list->entries[i].count[MONSTER_LIST_SECTION_LOS] > 0)
			list->total_entries[MONSTER_LIST_SECTION_LOS]++;

		if (list->entries[i].count[MONSTER_LIST_SECTION_ESP] > 0)
			list->total_entries[MONSTER_LIST_SECTION_ESP]++;

		list->total_monsters[MONSTER_LIST_SECTION_LOS] += list->entries[i].count[MONSTER_LIST_SECTION_LOS];
		list->total_monsters[MONSTER_LIST_SECTION_ESP] += list->entries[i].count[MONSTER_LIST_SECTION_ESP];
		list->distinct_entries++;
	}

	list->creation_turn = turn;
	list->sorted = FALSE;
}

/**
 * Standard comparison function for the monster list: sort by depth and then power.
 */
static int monster_list_standard_compare(const void *a, const void *b)
{
	const monster_race *ar = ((monster_list_entry_t *)a)->race;
	const monster_race *br = ((monster_list_entry_t *)b)->race;

	/* If this happens, something might be wrong in the collect function. */
	if (ar == NULL || br == NULL)
		return 1;

	/* Check depth first.*/
	if (ar->level > br->level)
		return -1;

	if (ar->level < br->level)
		return 1;

	/* Depths are equal, check power. */
	if (ar->power > br->power)
		return -1;

	if (ar->power < br->power)
		return 1;

	return 0;
}

/**
 * Sort the monster list with the given sort function.
 */
static void monster_list_sort(monster_list_t *list, int (*compare)(const void *, const void *))
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
	list->sorted = TRUE;
}

/**
 * Return an attribute to display a particular list entry with.
 *
 * \param entry is the monster list entry to display.
 * \return a term attribute for the monster entry.
 */
static byte monster_list_entry_line_attribute(const monster_list_entry_t *entry)
{
	/* Display uniques in a special colour */
	if (rf_has(entry->race->flags, RF_UNIQUE))
		return TERM_VIOLET;
	else if (entry->race->level > p_ptr->depth)
		return TERM_RED;
	else
		return TERM_WHITE;
}

/**
 * Format a section of the monster list: a header followed by monster list entry rows.
 *
 * This function will process each entry for the given section. It will display:
 * - monster char;
 * - number of monsters;
 * - monster name (truncated, if needed to fit the line);
 * - whether or not the monster is asleep (and how many if in a group);
 * - monster distance from the player (aligned to the right side of the list).
 * By passing in a NULL textblock, the maximum line width of the section can be found.
 *
 * \param list is the monster list to format.
 * \param tb is the textblock to produce or NULL if only the dimensions need to be calculated.
 * \param field is the section of the monster list to format.
 * \param lines_to_display are the number of entries to display (not including the header).
 * \param max_width is the maximum line width.
 * \param prefix is the beginning of the header; the remainder is appended with the number of monsters.
 * \param show_others is used to append "other monsters" to the header, after the number of monsters.
 * \param max_width_result is returned with the width needed to format the list without truncation.
 */
static void monster_list_format_section(const monster_list_t *list, textblock *tb, monster_list_section_t section, int lines_to_display, int max_width, const char *prefix, bool show_others, size_t *max_width_result)
{
	int remaining_monster_total = 0;
	int line_count = 0;
	int entry_index;
	int total;
	char line_buffer[200];
	const char *punctuation = (lines_to_display == 0) ? "." : ":";
	const char *others = (show_others) ? "other " : "";
	size_t max_line_length = 0;

	if (list == NULL || list->entries == NULL)
		return;

	total = list->distinct_entries;

	if (list->total_monsters[section] == 0) {
		max_line_length = strnfmt(line_buffer, sizeof(line_buffer), "%s no monsters.\n", prefix);

		if (tb != NULL)
			textblock_append(tb, "%s", line_buffer);

		/* Force a minimum width so that the prompt doesn't get cut off. */
		if (max_width_result != NULL)
			*max_width_result = MAX(max_line_length, 40);

		return;
	}

	max_line_length = strnfmt(line_buffer, sizeof(line_buffer), "%s %d %smonster%s%s\n", prefix, list->total_monsters[section], others, PLURAL(list->total_monsters[section]), punctuation);

	if (tb != NULL)
		textblock_append(tb, "%s", line_buffer);

	for (entry_index = 0; entry_index < total && line_count < lines_to_display; entry_index++) {
		char asleep[20] = { '\0' };
		char location[20] = { '\0' };
		byte line_attr;
		size_t full_width;
		size_t name_width;

		line_buffer[0] = '\0';

		if (list->entries[entry_index].count[section] == 0)
			continue;

		/* It doesn't make sense to display directions for more than one monster. */
		if (list->entries[entry_index].count[section] == 1) {
			char *direction1 = (list->entries[entry_index].dy <= 0) ? "N" : "S";
			char *direction2 = (list->entries[entry_index].dx <= 0) ? "W" : "E";
			strnfmt(location, sizeof(location), " %d %s %d %s", abs(list->entries[entry_index].dy), direction1, abs(list->entries[entry_index].dx), direction2);
		}

		/* Get width available for monster name and sleep tag: 2 for char and space; location includes padding; last -1 for some reason? */
		full_width = max_width - 2 - strlen(location) - 1;

		if (list->entries[entry_index].asleep[section] > 1)
			strnfmt(asleep, sizeof(asleep), " (%d asleep)", list->entries[entry_index].asleep[section]);
		else if (list->entries[entry_index].asleep[section] == 1)
			strnfmt(asleep, sizeof(asleep), " (asleep)");

		/* Clip the monster name to fit, and append the sleep tag. */
		name_width = MIN(full_width - strlen(asleep), sizeof(line_buffer));
		get_mon_name(line_buffer, name_width + 1, list->entries[entry_index].race, list->entries[entry_index].count[section]);
		my_strcat(line_buffer, asleep, sizeof(line_buffer));

		/* Calculate the width of the line for dynamic sizing; use a fixed max width for location and monster char. */
		max_line_length = MAX(max_line_length, strlen(line_buffer) + 12 + 2);

		/* textblock_append_pict will safely add the monster symbol, regardless of ASCII/graphics mode. */
		if (tb != NULL && tile_width == 1 && tile_height == 1) {
			textblock_append_pict(tb, list->entries[entry_index].attr, list->entries[entry_index].race->x_char);
			textblock_append(tb, " ");
		}

		/* Add the left-aligned and padded monster name which will align the location to the right. */
		if (tb != NULL) {
			/*
			 * Hack - Because monster race strings are UTF8, we have to add additional padding for
			 * any raw bytes that might be consolidated into one displayed character.
			 */
			full_width += strlen(line_buffer) - Term_mbstowcs(NULL, line_buffer, 0);
			line_attr = monster_list_entry_line_attribute(&list->entries[entry_index]);
			textblock_append_c(tb, line_attr, "%-*s%s\n", full_width, line_buffer, location);
		}

		line_count++;
	}

	/* Don't worry about the "...others" line, since it's probably shorter than what's already printed. */
	if (max_width_result != NULL)
		*max_width_result = max_line_length;

	/* Bail since we don't have enough room to display the remaining count or since we've displayed them all. */
	if (lines_to_display <= 0 || lines_to_display >= list->total_entries[section])
		return;

	/* Sum the remaining monsters, starting where we left off in the above loop. */
	while (entry_index < total) {
		remaining_monster_total += list->entries[entry_index].count[section];
		entry_index++;
	}

	if (tb != NULL)
		textblock_append(tb, "%6s...and %d others.\n", " ", remaining_monster_total);
}

/**
 * Allow the standard list formatted to be bypassed for special cases.
 *
 * Returning TRUE will bypass any other formatteding in monster_list_format_textblock().
 *
 * \param list is the monster list to format.
 * \param tb is the textblock to produce or NULL if only the dimensions need to be calculated.
 * \param max_lines is the maximum number of lines that can be displayed.
 * \param max_width is the maximum line width that can be displayed.
 * \param max_height_result is returned with the number of lines needed to format the list without truncation.
 * \param max_width_result is returned with the width needed to format the list without truncation.
 * \return TRUE if further formatting should be bypassed.
 */
static bool monster_list_format_special(const monster_list_t *list, textblock *tb, int max_lines, int max_width, size_t *max_height_result, size_t *max_width_result)
{
	if (p_ptr->timed[TMD_IMAGE] > 0) {
		/* Hack - message needs newline to calculate width properly. */
		const char *message = "Your hallucinations are too wild to see things clearly.\n";

		if (max_height_result != NULL)
			*max_height_result = 1;

		if (max_width_result != NULL)
			*max_width_result = strlen(message);

		if (tb != NULL)
			textblock_append_c(tb, TERM_ORANGE, "%s", message);

		return TRUE;
	}

	return FALSE;
}

/**
 * Format the entire monster list with the given parameters.
 *
 * This function can be used to calculate the preferred dimensions for the list
 * by passing in a NULL textblock. The LOS section of the list will always be
 * shown, while the other section will be added conditionally. Also, this function
 * calls monster_list_format_special() first; if that function returns TRUE, it
 * will bypass normal list formatting.
 *
 * \param list is the monster list to format.
 * \param tb is the textblock to produce or NULL if only the dimensions need to be calculated.
 * \param max_lines is the maximum number of lines that can be displayed.
 * \param max_width is the maximum line width that can be displayed.
 * \param max_height_result is returned with the number of lines needed to format the list without truncation.
 * \param max_width_result is returned with the width needed to format the list without truncation.
 */
static void monster_list_format_textblock(const monster_list_t *list, textblock *tb, int max_lines, int max_width, size_t *max_height_result, size_t *max_width_result)
{
	int header_lines = 1;
	int lines_remaining;
	int los_lines_to_display;
	int esp_lines_to_display;
	size_t max_los_line = 0;
	size_t max_esp_line = 0;

	if (list == NULL || list->entries == NULL)
		return;

	if (monster_list_format_special(list, tb, max_lines, max_width, max_height_result, max_width_result))
		return;

	los_lines_to_display = list->total_entries[MONSTER_LIST_SECTION_LOS];
	esp_lines_to_display = list->total_entries[MONSTER_LIST_SECTION_ESP];

	if (list->total_entries[MONSTER_LIST_SECTION_ESP] > 0)
		header_lines += 2;

	if (max_height_result != NULL)
		*max_height_result = header_lines + los_lines_to_display + esp_lines_to_display;

	lines_remaining = max_lines - header_lines - list->total_entries[MONSTER_LIST_SECTION_LOS];

	/* Remove ESP lines as needed. */
	if (lines_remaining < list->total_entries[MONSTER_LIST_SECTION_ESP])
		esp_lines_to_display = MAX(lines_remaining - 1, 0);

	/* If we don't even have enough room for the ESP header, start removing LOS lines, leaving one for the "...others". */
	if (lines_remaining < 0)
		los_lines_to_display = list->total_entries[MONSTER_LIST_SECTION_LOS] - abs(lines_remaining) - 1;

	/* Display only headers if we don't have enough space. */
	if (header_lines >= max_lines) {
		los_lines_to_display = 0;
		esp_lines_to_display = 0;
	}

	monster_list_format_section(list, tb, MONSTER_LIST_SECTION_LOS, los_lines_to_display, max_width, "You can see", FALSE, &max_los_line);

	if (list->total_entries[MONSTER_LIST_SECTION_ESP] > 0) {
		bool show_others = list->total_monsters[MONSTER_LIST_SECTION_LOS] > 0;

		if (tb != NULL)
			textblock_append(tb, "\n");

		monster_list_format_section(list, tb, MONSTER_LIST_SECTION_ESP, esp_lines_to_display, max_width, "You are aware of", show_others, &max_esp_line);
	}

	if (max_width_result != NULL)
		*max_width_result = MAX(max_los_line, max_esp_line);
}

/**
 * Display the monster list statically. This will force the list to be displayed to
 * the provided dimensions. Contents will be adjusted accordingly.
 *
 * In order to support more efficient monster flicker animations, this function uses
 * a shared list object so that it's not constantly allocating and freeing the list.
 *
 * \param height is the height of the list.
 * \param width is the width of the list.
 */
void monster_list_show_subwindow(int height, int width)
{
	textblock *tb = textblock_new();
	monster_list_t *list = monster_list_shared_instance();

	monster_list_reset(list);
	monster_list_collect(list);
	monster_list_sort(list, monster_list_standard_compare);

	/* Draw the list to exactly fit the subwindow. */
	monster_list_format_textblock(list, tb, height, width, NULL, NULL);
	textui_textblock_place(tb, SCREEN_REGION, NULL);

	textblock_free(tb);
}

/**
 * Display the monster list interactively. This will dynamically size the list for
 * the best appearance. This should only be used in the main term.
 *
 * \param height is the height limit for the list.
 * \param width is the width limit for the list.
 */
void monster_list_show_interactive(int height, int width)
{
	textblock *tb = textblock_new();
	monster_list_t *list = monster_list_new();
	size_t max_width = 0, max_height = 0;
	int safe_height, safe_width;
	region r;

	monster_list_collect(list);
	monster_list_sort(list, monster_list_standard_compare);

	/*
	 * Figure out optimal display rect. Large numbers are passed as the height and
	 * width limit so that we can calculate the maximum number of rows and columns
	 * to display the list nicely. We then adjust those values as needed to fit in
	 * the main term. Height is adjusted to account for the texblock prompt. The
	 * list is positioned on the right side of the term underneath the status line.
	 */
	monster_list_format_textblock(list, NULL, 1000, 1000, &max_height, &max_width);
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
	monster_list_format_textblock(list, tb, (int)max_height, safe_width, NULL, NULL);
	region_erase_bordered(&r);
	textui_textblock_show(tb, r, NULL);

	textblock_free(tb);
	monster_list_free(list);
}
