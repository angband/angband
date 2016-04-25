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
#include "parser.h"
#include "ui-visuals.h"
#include "z-color.h"
#include "z-util.h"

/* ----- Legacy Flicker Cycling ----- */

/**
 * Table to contain all of the color info for the flicker-style cycling.
 */
struct visuals_flicker {
	byte *cycles; /**< Array of \c max_cycles * \c colors_per_cycle colors. */
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
									  byte const attr)
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
static byte visuals_flicker_get_color(struct visuals_flicker *table,
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
 * \return An attribute in the flicker cycle, or \c COLOUR_DARK if an error
 *         occurred.
 */
byte visuals_flicker_get_attr_for_frame(byte const selection_attr,
										size_t const frame)
{
	size_t color_index = 0;

	if (visuals_flicker_table == NULL) {
		return COLOUR_DARK;
	}

	if (selection_attr >= visuals_flicker_table->max_cycles) {
		return COLOUR_DARK;
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
};

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

	if (strlen(color_code) == 0) {
		return PARSE_ERROR_INVALID_COLOR;
	}

	attr = color_char_to_attr(color_code[0]);

	if (attr < 0) {
		return PARSE_ERROR_INVALID_COLOR;
	}

	/* Set the search attribute for the following colors. */
	context->flicker_cycle_index = (byte)attr;
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

	if (strlen(color_code) == 0) {
		return PARSE_ERROR_INVALID_COLOR;
	}

	attr = color_char_to_attr(color_code[0]);

	if (attr < 0) {
		return PARSE_ERROR_INVALID_COLOR;
	}

	visuals_flicker_set_color(visuals_flicker_table,
							  context->flicker_cycle_index,
							  context->flicker_color_index,
							  (byte)attr);
	context->flicker_color_index++;

	return PARSE_ERROR_NONE;
}

/**
 * Create a new instance of the visuals parser.
 *
 * \return A parser or NULL if one could not be created.
 */
static struct parser *visuals_file_parser_init(void)
{
	struct visuals_parse_context *context = mem_zalloc(sizeof(*context));
	struct parser *parser = parser_new();

	if (context == NULL || parser == NULL) {
		return NULL;
	}

	context->flicker_color_index = 0;
	context->flicker_cycle_index = 0;

	parser_setpriv(parser, context);
	parser_reg(parser, "flicker sym color str name", visuals_parse_flicker);
	parser_reg(parser, "flicker-color sym color", visuals_parse_flicker_color);
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
	return parse_file_quit_not_found(parser, VISUALS_FILE_NAME);
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

	if (context != NULL) {
		mem_free(context);
	}

	parser_destroy(parser);
	return PARSE_ERROR_NONE;
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
}

/**
 * Visuals module registration.
 */
struct init_module ui_visuals_module = {
	.name = "ui-visuals",
	.init = ui_visuals_module_init,
	.cleanup = ui_visuals_module_cleanup
};
