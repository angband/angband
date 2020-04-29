/**
 * \file ui-entry-renderers.h
 * \brief Declare backend handling for parts of the second character screen
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
#ifndef INCLUDED_UI_ENTRY_RENDERERS_H
#define INCLUDED_UI_ENTRY_RENDERERS_H

#include "z-type.h"

struct ui_entry_details {
	/* This is the position for the first character in the label. */
	struct loc label_position;
	/* This is the position for the rendering of the first value. */
	struct loc value_position;
	/* This is the step size to use between values. */
	struct loc position_step;
	/* If true the characters of the label will be spaced vertically. */
	bool vertical_label;
	/*
	 * The rendering may alternate the colors for every other value shown.
	 * If this is true, the first value is shown with the alternate color.
	 */
	bool alternate_color_first;
	/*
	 * If true, the rune associated with the values is known to the
	 * player.
	 */
	bool known_rune;
};

/*
 * This is the value to use in vals or aux_vals array when the real value is
 * unknown to the player.
 */
#define UI_ENTRY_UNKNOWN_VALUE (INT_MAX)

/*
 * This is the value to use in vals or aux_vals array when the value is to
 * be treated as not present.
 */
#define UI_ENTRY_VALUE_NOT_PRESENT (INT_MAX - 1)

int ui_entry_renderer_get_min_index(void);
int ui_entry_renderer_get_index_limit(void);
const char *ui_entry_renderer_get_name(int ind);
int ui_entry_renderer_lookup(const char *name);
int ui_entry_renderer_query_value_width(int ind);
void ui_entry_renderer_apply(int ind, const wchar_t *label, int nlabel,
	const int *vals, const int *auxvals, int n,
	const struct ui_entry_details *details);
/* Handling for user-configurable parts */
int ui_entry_renderer_customize(int ind, const char *colors,
	const char *label_colors, const char *symbols);
char *ui_entry_renderer_get_colors(int ind);
char *ui_entry_renderer_get_label_colors(int ind);
char *ui_entry_renderer_get_symbols(int ind);
#endif /* INCLUDED_UI_ENTRY_RENDERERS_H */
