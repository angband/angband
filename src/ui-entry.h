/**
 * \file ui-entry.h
 * \brief Declarations to link object/player properties to 2nd character screen
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
#ifndef INCLUDED_UI_ENTRY_H
#define INCLUDED_UI_ENTRY_H

#include "h-basic.h"

struct player_ability;
struct object;
struct player;

struct ui_entry;
typedef bool (*ui_entry_predicate)(const struct ui_entry *entry, void *closure);
struct ui_entry_iterator;
struct cached_object_data;
struct cached_player_data;

int bind_object_property_to_ui_entry_by_name(const char *name, int type,
	int index, int value, bool have_value, bool isaux);
int bind_player_ability_to_ui_entry_by_name(const char *name,
	struct player_ability *ability, int value, bool have_value,
	bool isaux);

struct ui_entry_iterator *initialize_ui_entry_iterator(
	ui_entry_predicate predicate, void *closure, const char *sortcategory);
void release_ui_entry_iterator(struct ui_entry_iterator *i);
void reset_ui_entry_iterator(struct ui_entry_iterator *i);
int count_ui_entry_iterator(struct ui_entry_iterator *i);
struct ui_entry *advance_ui_entry_iterator(struct ui_entry_iterator *i);

bool ui_entry_has_category(const struct ui_entry *entry, const char *name);
void get_ui_entry_label(const struct ui_entry *entry, int length,
	bool pad_left, wchar_t *label);
int get_ui_entry_renderer_index(const struct ui_entry *entry);
bool is_ui_entry_for_known_rune(const struct ui_entry *entry,
	const struct player *p);
void compute_ui_entry_values_for_object(const struct ui_entry *entry,
	const struct object *obj, const struct player *p,
	struct cached_object_data **cache,  int *val, int *auxval);
void compute_ui_entry_values_for_player(const struct ui_entry *entry,
	struct player *p, struct cached_player_data **cache, int *val,
	int *auxval);
void release_cached_object_data(struct cached_object_data *cache);
void release_cached_player_data(struct cached_player_data *cache);

#endif /* INCLUDED_UI_ENTRY_H */
