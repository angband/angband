/**
 * \file obj-list.h
 * \brief Object list construction.
 *
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

#ifndef OBJECT_LIST_H
#define OBJECT_LIST_H

#define MAX_ITEMLIST 2560

typedef enum object_list_section_e {
	OBJECT_LIST_SECTION_LOS = 0,
	OBJECT_LIST_SECTION_NO_LOS,
	OBJECT_LIST_SECTION_MAX,
} object_list_section_t;

typedef struct object_list_entry_s {
	struct object *object;
	uint16_t count[OBJECT_LIST_SECTION_MAX];
	int16_t dx, dy;
} object_list_entry_t;

typedef struct object_list_s {
	object_list_entry_t *entries;
	size_t entries_size;
	uint16_t distinct_entries;
	int32_t creation_turn;
	uint16_t total_entries[OBJECT_LIST_SECTION_MAX];
	uint16_t total_objects[OBJECT_LIST_SECTION_MAX];
	bool sorted;
} object_list_t;

object_list_t *object_list_new(void);
void object_list_free(object_list_t *list);
void object_list_init(void);
void object_list_finalize(void);
object_list_t *object_list_shared_instance(void);
void object_list_reset(object_list_t *list);
void object_list_collect(object_list_t *list);
int object_list_standard_compare(const void *a, const void *b);
void object_list_sort(object_list_t *list,
					  int (*compare)(const void *, const void *));
uint8_t object_list_entry_line_attribute(const object_list_entry_t *entry);
void object_list_format_name(const object_list_entry_t *entry,
							 char *line_buffer, size_t size);

#endif /* OBJECT_LIST_H */
