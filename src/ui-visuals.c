/**
 * \file ui-visuals.c
 * \brief Appearance for screen elements.
 *
 * Copyright (c) 2016 Ben Semmler
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
#include "game-event.h"
#include "init.h"
#include "monster.h"
#include "parser.h"
#include "ui-visuals.h"
#include "z-color.h"
#include "z-util.h"

/** Max color cycle groups per cycler. */
static size_t const VISUALS_GROUPS_MAX = 8;

/** Max color cycles per group. */
static size_t const VISUALS_CYCLES_MAX = 64;

/** Max colors per color cycle. */
static size_t const VISUALS_STEPS_MAX = 32;

/** Value to mark unused or otherwise invalid colors in a color cycle. */
static uint8_t const VISUALS_INVALID_COLOR = 0xFF;

/* ----- Fancy Color Cycling ----- */

/**
 * A set of colors to rotate between.
 */
struct visuals_color_cycle {
	uint8_t *steps; /**< An array of color indexes. Colors will be cycled in this order. */
	size_t max_steps; /**< The size of the \c steps array. */
	char *cycle_name; /**< The identifier of the color cycle. */
	uint8_t invalid_color; /**< A value to mark locations in \c steps that should not be considered a valid color. */
};

/**
 * Create a new color cycle.
 *
 * \param name The identifier for the new color cycle.
 * \param step_count The maximum number of steps in the cycle.
 * \param invalid_color A value used to mark steps that should not be considered
 *        valid colors. This is also used as the initial value for each value in
 *        the \c steps array.
 * \return A new color cycle instance or NULL if an error occurred.
 */
static struct visuals_color_cycle *visuals_color_cycle_new(const char *name,
														   size_t const step_count,
														   uint8_t const invalid_color)
{
	struct visuals_color_cycle *cycle = NULL;

	if (step_count == 0) {
		return NULL;
	}

	cycle = mem_zalloc(sizeof(*cycle));

	if (cycle == NULL) {
		return NULL;
	}

	cycle->steps = mem_zalloc(step_count * sizeof(*(cycle->steps)));

	if (cycle->steps == NULL) {
		mem_free(cycle);
		return NULL;
	}

	memset(cycle->steps, invalid_color, step_count);
	cycle->invalid_color = invalid_color;
	cycle->max_steps = step_count;
	cycle->cycle_name = string_make(name);
	return cycle;
}

/**
 * Deallocate a color cycle.
 *
 * \param cycle The color cycle to deallocate.
 */
static void visuals_color_cycle_free(struct visuals_color_cycle *cycle)
{
	if (cycle == NULL) {
		return;
	}

	if (cycle->steps != NULL) {
		mem_free(cycle->steps);
		cycle->steps = NULL;
	}

	if (cycle->cycle_name != NULL) {
		string_free(cycle->cycle_name);
		cycle->cycle_name = NULL;
	}

	mem_free(cycle);
	cycle = NULL;
}

/**
 * Copy a given color cycle by creating a new instance with the same values. If
 * there are any invalid colors in the original, the copy will contain only the
 * valid colors in the original (preserving the original order).
 *
 * \param original The color cycle to copy.
 * \return A new color cycle instance, containing the same cycle information as
 *         \c original. NULL is returned on error.
 */
static struct visuals_color_cycle *visuals_color_cycle_copy(struct visuals_color_cycle const *original)
{
	struct visuals_color_cycle *copy = NULL;
	size_t step = 0;
	size_t valid_colors = 0;
	size_t copy_step = 0;

	if (original == NULL) {
		return NULL;
	}

	/* Find the actual number of colors being used by the original cycle. */
	for (step = 0; step < original->max_steps; step++) {
		if (original->steps[step] != original->invalid_color	) {
			valid_colors++;
		}
	}

	copy = visuals_color_cycle_new(original->cycle_name,
								   valid_colors,
								   original->invalid_color);

	if (copy == NULL) {
		return NULL;
	}

	for (step = 0; step < original->max_steps && copy_step < valid_colors; step++) {
		if (original->steps[step] != original->invalid_color) {
			copy->steps[copy_step] = original->steps[step];
			copy_step++;
		}
	}

	return copy;
}

/**
 * Return the next color in the cycle for a given frame.
 *
 * \param cycle The color cycle to select a color from.
 * \param frame An arbitrary value used to select the color step.
 * \return A color or \c BASIC_COLORS if an error occurred.
 */
static uint8_t visuals_color_cycle_attr_for_frame(struct visuals_color_cycle const *cycle,
											   size_t const frame)
{
	size_t step = 0;

	if (cycle == NULL) {
		return BASIC_COLORS;
	}

	step = frame % cycle->max_steps;
	return cycle->steps[step];
}

/**
 * A group of color cycles. This is a way of keeping related color cycles
 * together, to provide more flexibility in the color cycling system.
 */
struct visuals_cycle_group {
	struct visuals_color_cycle **cycles; /**< An array of color cycle references. */
	size_t max_cycles; /**< The size of the \c cycles array. */
	char *group_name; /**< The identifier of the group. */
};

/**
 * Create a new color cycle group.
 *
 * \param name The identifier for the group.
 * \param cycle_count The maximum number of color cycles that will be in the
 *        group.
 * \return A new color cycle group or NULL if an error occurred.
 */
static struct visuals_cycle_group *visuals_cycle_group_new(const char *name,
														   size_t const cycle_count)
{
	struct visuals_cycle_group *group = NULL;

	if (cycle_count == 0) {
		return NULL;
	}

	group = mem_zalloc(sizeof(*group));

	if (group == NULL) {
		return NULL;
	}

	group->cycles = mem_zalloc(cycle_count * sizeof(*(group->cycles)));

	if (group->cycles == NULL) {
		mem_free(group);
		return NULL;
	}

	group->max_cycles = cycle_count;
	group->group_name = string_make(name);
	return group;
}

/**
 * Deallocate a color cycle group.
 *
 * \param group The group to deallocate.
 */
static void visuals_cycle_group_free(struct visuals_cycle_group *group)
{
	size_t i = 0;

	if (group == NULL) {
		return;
	}

	if (group->cycles != NULL) {
		for (i = 0; i < group->max_cycles; i++) {
			visuals_color_cycle_free(group->cycles[i]);
		}

		mem_free(group->cycles);
		group->cycles = NULL;
	}

	if (group->group_name != NULL) {
		string_free(group->group_name);
		group->group_name = NULL;
	}

	mem_free(group);
	group = NULL;
}

/**
 * A color cycling table, made up of groups of actual color cycles.
 */
struct visuals_cycler {
	struct visuals_cycle_group **groups; /**< An array of references to groups of color cyclers. */
	size_t max_groups; /**< Size of the \c groups array. */
};

/**
 * Module color cycling table instance.
 */
static struct visuals_cycler *visuals_cycler_table = NULL;

/**
 * Create a new color cycling table.
 *
 * \param group_count The maximum number of groups that the table will contain.
 * \return A new color cycling table or NULL if an error occurred.
 */
static struct visuals_cycler *visuals_cycler_new(size_t const group_count)
{
	struct visuals_cycler *cycler = NULL;

	if (group_count == 0) {
		return NULL;
	}

	cycler = mem_zalloc(sizeof(*cycler));

	if (cycler == NULL) {
		return NULL;
	}

	cycler->groups = mem_zalloc(group_count * sizeof(*(cycler->groups)));

	if (cycler->groups == NULL) {
		mem_free(cycler);
		return NULL;
	}

	cycler->max_groups = group_count;
	return cycler;
}

/**
 * Deallocate a color cycler.
 *
 * \param cycler The cycler to deallocate.
 */
static void visuals_cycler_free(struct visuals_cycler *cycler)
{
	size_t i = 0;

	if (cycler == NULL) {
		return;
	}

	if (cycler->groups != NULL) {
		for (i = 0; i < cycler->max_groups; i++) {
			visuals_cycle_group_free(cycler->groups[i]);
		}

		mem_free(cycler->groups);
		cycler->groups = NULL;
	}

	mem_free(cycler);
	cycler = NULL;
}

/**
 * Search for a color cycle with the given group and name.
 *
 * \param cycler The cycler to search.
 * \param group_name The name of the group to search.
 * \param cycle_name The name of the color cycle to find.
 * \return A color cycle or NULL if an error occurred.
 */
static struct visuals_color_cycle *visuals_cycler_cycle_by_name(struct visuals_cycler const *cycler,
																const char *group_name,
																const char *cycle_name)
{
	struct visuals_cycle_group *group = NULL;
	struct visuals_color_cycle *cycle = NULL;
	size_t i = 0;

	if (group_name == NULL || strlen(group_name) == 0) {
		return NULL;
	}

	if (cycle_name == NULL || strlen(cycle_name) == 0) {
		return NULL;
	}

	for (i = 0; i < cycler->max_groups; i++) {
		if (streq(cycler->groups[i]->group_name, group_name)) {
			group = cycler->groups[i];
			break;
		}
	}

	if (group == NULL) {
		return NULL;
	}

	for (i = 0; i < group->max_cycles; i++) {
		if (streq(group->cycles[i]->cycle_name, cycle_name)) {
			cycle = group->cycles[i];
			break;
		}
	}

	return cycle;
}

/**
 * Get an attribute for a color cycle in a color cycle group from the module
 * table.
 *
 * \param group_name The color cycle group to search.
 * \param cycle_name The color cycle to use.
 * \param frame An arbitrary value used to select which color in the flicker
 *        cycle to return.
 * \return An attribute in the color cycle, or \c BASIC_COLORS if an error
 *         occurred.
 */
uint8_t visuals_cycler_get_attr_for_frame(const char *group_name,
									   const char *cycle_name,
									   size_t const frame)
{
	struct visuals_cycler *table = visuals_cycler_table;
	struct visuals_color_cycle *cycle = NULL;
	cycle = visuals_cycler_cycle_by_name(table, group_name, cycle_name);

	if (cycle == NULL) {
		return BASIC_COLORS;
	}

	return visuals_color_cycle_attr_for_frame(cycle, frame);
}

/** A table to quickly map monster races to color cycles. */
struct {
	struct visuals_color_cycle **race;
	size_t max_entries;
	size_t alloc_size;
} *visuals_color_cycles_by_race = NULL;

/**
 * Set a color cycle for a monster race. If a matching color cycle cannot be
 * found, the monster race will not be color cycled.
 *
 * When this module is set up, we don't know how many monster races there will
 * be, nor is there a maximum permitted number of races. This function will
 * reallocate the \c visuals_color_cycles_by_race table as needed, using its
 * allocation increment. This table is initially created in the module init
 * function.
 *
 * \param race The monster race to set the color cycle for.
 * \param group_name The group of the preferred color cycle.
 * \param cycle_name The name of the preferred color cycle.
 */
void visuals_cycler_set_cycle_for_race(struct monster_race const *race,
									   const char *group_name,
									   const char *cycle_name)
{
	struct visuals_cycler *table = visuals_cycler_table;
	struct visuals_color_cycle *cycle = NULL;

	if (race == NULL || group_name == NULL || cycle_name == NULL) {
		return;
	}

	if (visuals_color_cycles_by_race == NULL) {
		return;
	}

	while (race->ridx >= visuals_color_cycles_by_race->max_entries) {
		/* Keep reallocating until we can fit the index. This for the case when
		 * indexes may not be parsed in numerical order. */
		size_t old_count = visuals_color_cycles_by_race->max_entries;
		size_t new_count = old_count + visuals_color_cycles_by_race->alloc_size;
		size_t new_size = new_count * sizeof(*(visuals_color_cycles_by_race->race));
		visuals_color_cycles_by_race->race = mem_realloc(visuals_color_cycles_by_race->race, new_size);
		visuals_color_cycles_by_race->max_entries = new_count;

		if (new_count >= 10000) {
			/* Prevent the list from growing to a ridiculous size. */
			quit("Allocated too many color cycle/race refs. Check monster info?");
		}
	}

	cycle = visuals_cycler_cycle_by_name(table, group_name, cycle_name);

	if (cycle == NULL) {
		return;
	}

	visuals_color_cycles_by_race->race[race->ridx] = cycle;
}

/**
 * Get an attribute from a monster race's color cycle.
 *
 * \param race The race to get an attribute for.
 * \param frame An arbitrary value used to select which color in the flicker
 *        cycle to return.
 * \return An attribute in the color cycle, or \c BASIC_COLORS if an error
 *         occurred.
 */
uint8_t visuals_cycler_get_attr_for_race(struct monster_race const *race,
									  size_t const frame)
{
	struct visuals_color_cycle *cycle = NULL;

	if (race == NULL) {
		return BASIC_COLORS;
	}

	if (visuals_color_cycles_by_race == NULL ||
		race->ridx >= visuals_color_cycles_by_race->max_entries) {
		return BASIC_COLORS;
	}

	cycle = visuals_color_cycles_by_race->race[race->ridx];

	if (cycle == NULL) {
		return BASIC_COLORS;
	}

	return visuals_color_cycle_attr_for_frame(cycle, frame);
}

/* ----- Legacy Flicker Cycling ----- */

/**
 * Table to contain all of the color info for the flicker-style cycling.
 */
struct visuals_flicker {
	uint8_t *cycles; /**< Array of \c max_cycles * \c colors_per_cycle colors. */
	size_t max_cycles; /**< Maximum number of cycles in this table; not all may be used. */
	size_t colors_per_cycle; /**< Maximum number of steps in each cycle. */
};

/**
 * Module flicker color table instance.
 */
static struct visuals_flicker *visuals_flicker_table = NULL;

/**
 * Allocate a new flicker color table to hold a fixed number of entries.
 *
 * \param max_cycles The maximum number of cycles to allow for this table.
 * \param colors_per_cycle The maximum number of color steps in each cycle.
 * \return A new flicker color table instance, or NULL if one could not be
 *         created.
 */
static struct visuals_flicker *visuals_flicker_new(size_t const max_cycles,
												   size_t const colors_per_cycle)
{
	struct visuals_flicker *table = NULL;
	size_t cycles_size = 0;

	if (max_cycles < BASIC_COLORS || colors_per_cycle == 0) {
		/* Make sure we can at least support the basic color table. */
		return NULL;
	}

	table = mem_zalloc(sizeof(*table));

	if (table == NULL) {
		return NULL;
	}

	cycles_size = max_cycles * colors_per_cycle * sizeof(*(table->cycles));
	table->cycles = mem_zalloc(cycles_size);

	if (table->cycles == NULL) {
		mem_free(table);
		return NULL;
	}

	table->max_cycles = max_cycles;
	table->colors_per_cycle	= colors_per_cycle;
	return table;
}

/**
 * Deallocate a flicker color table.
 *
 * \param table The color table to deallocate.
 */
static void visuals_flicker_free(struct visuals_flicker *table)
{
	if (table == NULL) {
		return;
	}

	if (table->cycles != NULL) {
		mem_free(table->cycles);
		table->cycles = NULL;
	}

	mem_free(table);
	table = NULL;
}

/**
 * Set the color that should appear at a particular step in a cycle. If any of
 * the indexes are out of range, the color is ignored.
 *
 * \param table The color table to update.
 * \param cycle_index The index of the cycle to update.
 * \param color_index The step in the given cycle to update.
 * \param attr The actual color to use.
 */
static void visuals_flicker_set_color(struct visuals_flicker *table,
		size_t const cycle_index,
		size_t const color_index,
		uint8_t const attr)
{
	if (table == NULL) {
		return;
	}

	if (cycle_index >= table->max_cycles) {
		return;
	}

	if (color_index >= table->colors_per_cycle) {
		return;
	}

	table->cycles[(cycle_index * table->colors_per_cycle) + color_index] = attr;
}

/**
 * Get the color that should appear at a particular step in a cycle. If any of
 * the indexes are out of range, the color with index 0 is returned.
 *
 * \param table The color table to use.
 * \param cycle_index The index of the desired cycle.
 * \param color_index The desired step in the cycle.
 * \return The color at the given step in the given cycle.
 */
static uint8_t visuals_flicker_get_color(struct visuals_flicker *table,
									  size_t const cycle_index,
									  size_t const color_index)
{
	if (table == NULL) {
		return 0;
	}

	if (cycle_index >= table->max_cycles) {
		return 0;
	}

	if (color_index >= table->colors_per_cycle) {
		return 0;
	}

	return table->cycles[(cycle_index * table->colors_per_cycle) + color_index];
}

/**
 * Get an attribute for the flicker cycle for a base attribute from the module
 * color table.
 *
 * \param selection_attr The attribute used to select which flicker cycle to
 *        use.
 * \param frame An arbitrary value used to select which color in the flicker
 *        cycle to return.
 * \return An attribute in the flicker cycle, or \c BASIC_COLORS if an error
 *         occurred.
 */
uint8_t visuals_flicker_get_attr_for_frame(uint8_t const selection_attr,
		size_t const frame)
{
	size_t color_index = 0;

	if (visuals_flicker_table == NULL) {
		return BASIC_COLORS;
	}

	if (selection_attr >= visuals_flicker_table->max_cycles) {
		return BASIC_COLORS;
	}

	color_index = frame % visuals_flicker_table->colors_per_cycle;
	return visuals_flicker_get_color(visuals_flicker_table,
									 selection_attr,
									 color_index);
}

/* ----- Visuals Parsing ----- */

/**
 * Context data for file parsing.
 */
struct visuals_parse_context {
	size_t flicker_cycle_index;
	size_t flicker_color_index;

	struct visuals_color_cycle **cycles; /**< Newly allocated cycle instances. */
	size_t max_cycles;
	size_t cycles_index;
	size_t cycle_step_index;

	char **group_names;
	size_t max_groups;

	struct visuals_color_cycle **group_cycles; /**< References to cycles in the group. */
	size_t max_group_cycles;
};

/**
 * Allocate a visuals parser context.
 *
 * \return A parser context or NULL if one could not be created.
 */
static struct visuals_parse_context *visuals_parse_context_new(void)
{
	struct visuals_parse_context *context = mem_zalloc(sizeof(*context));

	if (context == NULL) {
		return NULL;
	}

	/* Add some extra capacity, since this array will start at 1 instead of 0. */
	context->max_cycles = VISUALS_GROUPS_MAX * VISUALS_CYCLES_MAX + 1;
	context->cycles = mem_zalloc(context->max_cycles * sizeof(*(context->cycles)));

	if (context->cycles == NULL) {
		mem_free(context);
		return NULL;
	}

	context->max_groups = VISUALS_GROUPS_MAX;
	context->group_names = mem_zalloc(context->max_groups * sizeof(*(context->group_names)));

	if (context->group_names == NULL) {
		mem_free(context->cycles);
		mem_free(context);
		return NULL;
	}

	context->max_group_cycles = context->max_groups * VISUALS_CYCLES_MAX;
	context->group_cycles = mem_zalloc(context->max_group_cycles * sizeof(*(context->group_cycles)));

	if (context->group_cycles == NULL) {
		mem_free(context->group_names);
		mem_free(context->cycles);
		mem_free(context);
		return NULL;
	}

	context->cycles_index = 0;
	context->cycle_step_index = 0;
	context->flicker_color_index = 0;
	context->flicker_cycle_index = 0;
	return context;
}

/**
 * Deallocate parser context.
 *
 * \param context The parser context to free.
 */
static void visuals_parse_context_free(struct visuals_parse_context *context)
{
	size_t i = 0;

	if (context == NULL) {
		return;
	}

	if (context->cycles != NULL) {
		for (i = 0; i < context->max_cycles; i++) {
			visuals_color_cycle_free(context->cycles[i]);
		}

		mem_free(context->cycles);
		context->cycles = NULL;
	}

	if (context->group_names != NULL) {
		for (i = 0; i < context->max_groups; i++) {
			string_free(context->group_names[i])	;
		}

		mem_free(context->group_names);
		context->group_names = NULL;
	}

	if (context->group_cycles != NULL) {
		/* This array is only references and not instances. */
		mem_free(context->group_cycles);
		context->group_cycles = NULL;
	}

	mem_free(context);
	context = NULL;
}

/**
 * Create a cycler instance from the parser context.
 *
 * \param context The current visuals parser context.
 * \return A new cycler instance or NULL if an error occurred.
 */
static struct visuals_cycler *visuals_parse_context_convert(struct visuals_parse_context const *context)
{
	size_t group_count = 0;
	size_t group = 0;
	struct visuals_cycler *cycler = NULL;

	if (context == NULL) {
		return NULL;
	}

	/* Count the number of groups, based only on group name. */
	while (context->group_names[group_count] != NULL) {
		group_count++;
	}

	cycler = visuals_cycler_new(group_count);

	if (cycler == NULL) {
		return NULL;
	}

	for (group = 0; group < group_count; group++) {
		size_t cycle_count = 0;
		size_t const cycle_offset = group * VISUALS_CYCLES_MAX;
		struct visuals_cycle_group *cycle_group = NULL;
		size_t cycle = 0;

		/* Create a new group based with the number of actual cyles we have. */
		while(context->group_cycles[cycle_offset + cycle_count] != NULL) {
			cycle_count++;
		}

		cycle_group = visuals_cycle_group_new(context->group_names[group], cycle_count);

		if (cycle_group == NULL) {
			return NULL;
		}

		/* Copy all of the cycles over to the group. */
		for (cycle = 0; cycle < cycle_count; cycle++) {
			struct visuals_color_cycle *old_cycle = context->group_cycles[cycle_offset + cycle];
			cycle_group->cycles[cycle] = visuals_color_cycle_copy(old_cycle);

			if (cycle_group->cycles[cycle] == NULL) {
				return NULL;
			}
		}

		cycler->groups[group] = cycle_group;
	}

	return cycler;
}

/**
 * Handle a "flicker" row.
 *
 * \param parser The visuals parser.
 * \return A parser error code.
 */
static enum parser_error visuals_parse_flicker(struct parser *parser)
{
	struct visuals_parse_context *context = parser_priv(parser);
	const char *color_code = NULL;
	int attr = 0;

	if (context == NULL) {
		return PARSE_ERROR_INTERNAL;
	}

	color_code = parser_getsym(parser, "color");

	if (color_code == NULL || strlen(color_code) == 0) {
		return PARSE_ERROR_INVALID_COLOR;
	}

	attr = color_char_to_attr(color_code[0]);

	if (attr < 0) {
		return PARSE_ERROR_INVALID_COLOR;
	}

	/* Set the search attribute for the following colors. */
	context->flicker_cycle_index = (uint8_t)attr;
	context->flicker_color_index = 0;

	return PARSE_ERROR_NONE;
}

/**
 * Handle a "flicker-color" row.
 *
 * \param parser The visuals parser.
 * \return A parser error code.
 */
static enum parser_error visuals_parse_flicker_color(struct parser *parser)
{
	struct visuals_parse_context *context = parser_priv(parser);
	const char *color_code = NULL;
	int attr = 0;

	if (context == NULL) {
		return PARSE_ERROR_INTERNAL;
	}

	color_code = parser_getsym(parser, "color");

	if (color_code == NULL || strlen(color_code) == 0) {
		return PARSE_ERROR_INVALID_COLOR;
	}

	attr = color_char_to_attr(color_code[0]);

	if (attr < 0) {
		return PARSE_ERROR_INVALID_COLOR;
	}

	visuals_flicker_set_color(visuals_flicker_table,
							  context->flicker_cycle_index,
							  context->flicker_color_index,
							  (uint8_t)attr);
	context->flicker_color_index++;

	return PARSE_ERROR_NONE;
}

/**
 * Handle a "cycle" row.
 *
 * \param parser The visuals parser.
 * \return A parser error code.
 */
static enum parser_error visuals_parse_cycle(struct parser *parser)
{
	struct visuals_parse_context *context = parser_priv(parser);
	const char *parsed_group = NULL;
	const char *parsed_name = NULL;
	size_t i = 0;
	size_t current_group_index = 0;

	if (context == NULL) {
		return PARSE_ERROR_INTERNAL;
	}

	/* Move to the next available slot in the cycle list. This will start the
	 * list at 1 (instead of zero), but we need to use cycles_index to keep
	 * track of which cycle we will be adding colors to. */
	context->cycles_index++;

	if (context->cycles_index >= context->max_cycles) {
		return PARSE_ERROR_TOO_MANY_ENTRIES;
	}

	parsed_group = parser_getsym(parser, "group");

	if (parsed_group == NULL || strlen(parsed_group) == 0) {
		return PARSE_ERROR_INVALID_VALUE;
	}

	parsed_name = parser_getsym(parser, "name");

	if (parsed_name == NULL || strlen(parsed_name) == 0) {
		return PARSE_ERROR_INVALID_VALUE;
	}

	/* Create a new cycle with the maximum number of steps (if we can),
	 * otherwise bail. */
	if (context->cycles[context->cycles_index] == NULL) {
		context->cycles[context->cycles_index] = visuals_color_cycle_new(parsed_name,
																		 VISUALS_STEPS_MAX,
																		 VISUALS_INVALID_COLOR);

		if (context->cycles[context->cycles_index] == NULL) {
			return PARSE_ERROR_INTERNAL;
		}
	}

	/* Find the group that matches the parsed cycle's group or create a new one
	 * if not found. */
	current_group_index = context->max_groups;

	for (i = 0; i < context->max_groups; i++) {
		if (context->group_names[i] == NULL) {
			context->group_names[i] = string_make(parsed_group);
			current_group_index = i;
			break;
		}
		else {
			const char *existing_group = context->group_names[i];

			if (existing_group != NULL && streq(parsed_group, existing_group)) {
				current_group_index = i;
				break;
			}
		}
	}

	/* Assign the parsed cycle to its group. If a cycle with a matching name is
	 * found, the parsed cycle replaces that one, otherwise, it is added to the
	 * end of the group. */
	for (i = 0; i < VISUALS_CYCLES_MAX; i++) {
		size_t offset = current_group_index * VISUALS_CYCLES_MAX + i;

		if (offset >= context->max_group_cycles) {
			continue;
		}

		if (context->group_cycles[offset] == NULL) {
			context->group_cycles[offset] = context->cycles[context->cycles_index];
			break;
		}
		else {
			const char *existing_name = context->group_cycles[offset]->cycle_name;

			if (existing_name != NULL && streq(parsed_name, existing_name)) {
				context->group_cycles[offset] = context->cycles[context->cycles_index];
				break;
			}
		}
	}

	/* Reset the step index before we start adding colors to the cycle. */
	context->cycle_step_index = 0;

	return PARSE_ERROR_NONE;
}

/**
 * Handle a "cycle-color" row.
 *
 * \param parser The visuals parser.
 * \return A parser error code.
 */
static enum parser_error visuals_parse_cycle_color(struct parser *parser)
{
	struct visuals_parse_context *context = parser_priv(parser);
	const char *color_code = NULL;
	int attr = 0;

	if (context == NULL) {
		return PARSE_ERROR_INTERNAL;
	}

	if (context->cycle_step_index >= context->cycles[context->cycles_index]->max_steps) {
		return PARSE_ERROR_TOO_MANY_ENTRIES;
	}

	color_code = parser_getsym(parser, "color");

	if (color_code == NULL || strlen(color_code) == 0) {
		return PARSE_ERROR_INVALID_COLOR;
	}

	attr = color_char_to_attr(color_code[0]);

	if (attr < 0) {
		return PARSE_ERROR_INVALID_COLOR;
	}

	context->cycles[context->cycles_index]->steps[context->cycle_step_index] = attr;
	context->cycle_step_index++;

	return PARSE_ERROR_NONE;
}

/**
 * Create a new instance of the visuals parser.
 *
 * \return A parser or NULL if one could not be created.
 */
static struct parser *visuals_file_parser_init(void)
{
	struct visuals_parse_context *context = visuals_parse_context_new();
	struct parser *parser = parser_new();

	if (context == NULL || parser == NULL) {
		return NULL;
	}

	parser_setpriv(parser, context);
	parser_reg(parser, "flicker sym color str name", visuals_parse_flicker);
	parser_reg(parser, "flicker-color sym color", visuals_parse_flicker_color);
	parser_reg(parser, "cycle sym group sym name", visuals_parse_cycle);
	parser_reg(parser, "cycle-color sym color", visuals_parse_cycle_color);
	return parser;
}

/**
 * Run a parser for a file parser.
 *
 * \param parser The visuals parser.
 * \return A parser error code.
 */
static errr visuals_file_parser_run(struct parser *parser)
{
	return parse_file_quit_not_found(parser, "visuals");
}

/**
 * Clean up the visuals parser itself.
 *
 * \param parser The visuals parser.
 * \return A parser error code.
 */
static errr visuals_file_parser_finish(struct parser *parser)
{
	struct visuals_parse_context *context = parser_priv(parser);
	visuals_cycler_table = visuals_parse_context_convert(context);
	visuals_parse_context_free(context);
	parser_destroy(parser);
	return (visuals_cycler_table == NULL) ? PARSE_ERROR_INTERNAL : PARSE_ERROR_NONE;
}

/**
 * Perform any final things needed after parsing the visuals file.
 */
static void visuals_file_parser_cleanup(void)
{
	/* Stub for now. */
}

/**
 * File parser instance for the visuals file.
 */
static struct file_parser visuals_file_parser = {
	.name = "visuals",
	.init = visuals_file_parser_init,
	.run = visuals_file_parser_run,
	.finish = visuals_file_parser_finish,
	.cleanup = visuals_file_parser_cleanup
};

/* ----- UI Visuals Module ----- */

/**
 * Set up the visuals module. This includes parsing the visuals file.
 */
static void ui_visuals_module_init(void)
{
	event_signal_message(EVENT_INITSTATUS, 0, "Initializing visuals");

	/* Use the same values as the legacy flicker table. */
	visuals_flicker_table = visuals_flicker_new(MAX_COLORS, 3);

	if (visuals_flicker_table == NULL) {
		quit("Unable to allocate flicker table");
	}

	/* Allocate a lookup table for race to color cycles. */
	visuals_color_cycles_by_race = mem_zalloc(sizeof(*visuals_color_cycles_by_race));

	if (visuals_color_cycles_by_race == NULL) {
		quit("Unable to allocate race/color cycle table");
	}

	/* At this point, we don't know how many entries we'll need, so the table
	 * will be resized dynamically. */
	visuals_color_cycles_by_race->max_entries = 0;
	visuals_color_cycles_by_race->alloc_size = 100;

	/* Parsing will result in the cycler table being set up. */
	if (run_parser(&visuals_file_parser)) {
		quit("Cannot initialize visuals");
	}

	if (visuals_file_parser.cleanup) {
		visuals_file_parser.cleanup();
	}
}

/**
 * Tear down the visuals module.
 */
static void ui_visuals_module_cleanup(void)
{
	visuals_flicker_free(visuals_flicker_table);
	visuals_cycler_free(visuals_cycler_table);

	if (visuals_color_cycles_by_race != NULL) {
		if (visuals_color_cycles_by_race->race != NULL) {
			mem_free(visuals_color_cycles_by_race->race);
			visuals_color_cycles_by_race->race = NULL;
		}

		mem_free(visuals_color_cycles_by_race);
		visuals_color_cycles_by_race = NULL;
	}
}

/**
 * Visuals module registration.
 */
struct init_module ui_visuals_module = {
	.name = "ui-visuals",
	.init = ui_visuals_module_init,
	.cleanup = ui_visuals_module_cleanup
};
