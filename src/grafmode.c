/**
 * \file grafmode.c
 * \brief Load a list of possible graphics modes.
 *
 * Copyright (c) 2011 Brett Reid
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband license":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"
#include "datafile.h"
#include "grafmode.h"
#include "init.h"

graphics_mode *graphics_modes;
graphics_mode *current_graphics_mode = NULL;
int graphics_mode_high_id;

static enum parser_error parse_graf_name(struct parser *p) {
	graphics_mode *list = parser_priv(p);
	graphics_mode *mode = mem_zalloc(sizeof(graphics_mode));
	if (!mode) {
		return PARSE_ERROR_OUT_OF_MEMORY;
	}
	mode->pNext = list;
	mode->grafID = parser_getuint(p, "index");
	my_strcpy(mode->menuname, parser_getstr(p, "menuname"), 32);

	mode->alphablend = 0;
	mode->overdrawRow = 0;
	mode->overdrawMax = 0;
	my_strcpy(mode->file, "", 32);
	my_strcpy(mode->pref, "none", 32);
	
	parser_setpriv(p, mode);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_graf_directory(struct parser *p) {
	graphics_mode *mode = parser_priv(p);
	const char *dir = parser_getsym(p, "dirname");
	if (!mode) {
		return PARSE_ERROR_INVALID_VALUE;
	}

	/* Build a usable path */
	path_build(mode->path, sizeof(mode->path), ANGBAND_DIR_TILES, dir);

	return PARSE_ERROR_NONE;
}

static enum parser_error parse_graf_size(struct parser *p) {
	graphics_mode *mode = parser_priv(p);
	if (!mode) {
		return PARSE_ERROR_INVALID_VALUE;
	}
	mode->cell_width = parser_getuint(p, "wid");
	mode->cell_height = parser_getuint(p, "hgt");
	my_strcpy(mode->file, parser_getstr(p, "filename"), 32);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_graf_pref(struct parser *p) {
	graphics_mode *mode = parser_priv(p);
	if (!mode) {
		return PARSE_ERROR_INVALID_VALUE;
	}
	my_strcpy(mode->pref, parser_getstr(p, "prefname"), 32);
	return PARSE_ERROR_NONE;
}

static enum parser_error parse_graf_extra(struct parser *p) {
	graphics_mode *mode = parser_priv(p);
	if (!mode) {
		return PARSE_ERROR_INVALID_VALUE;
	}
	mode->alphablend = parser_getuint(p, "alpha");
	mode->overdrawRow = parser_getuint(p, "row");
	mode->overdrawMax = parser_getuint(p, "max");
	return PARSE_ERROR_NONE;
}

static struct parser *init_parse_grafmode(void) {
	struct parser *p = parser_new();
	parser_setpriv(p, NULL);

	parser_reg(p, "name uint index str menuname", parse_graf_name);
	parser_reg(p, "directory sym dirname", parse_graf_directory);
	parser_reg(p, "size uint wid uint hgt str filename", parse_graf_size);
	parser_reg(p, "pref str prefname", parse_graf_pref);
	parser_reg(p, "extra uint alpha uint row uint max", parse_graf_extra);

	return p;
}

static errr finish_parse_grafmode(struct parser *p) {
	graphics_mode *mode, *n;
	int max = 0;
	int count = 0;
	int i;
	
	/* See how many graphics modes we have and what the highest index is */
	if (p) {
		mode = parser_priv(p);
		while (mode) {
			if (mode->grafID > max) {
				max = mode->grafID;
			}
			count++;
			mode = mode->pNext;
		}
	}

	/* Copy the loaded modes to the global variable */
	if (graphics_modes) {
		close_graphics_modes();
	}

	graphics_modes = mem_zalloc(sizeof(graphics_mode) * (count+1));
	if (p) {
		mode = parser_priv(p);
		for (i = count-1; i >= 0; i--, mode = mode->pNext) {
			memcpy(&(graphics_modes[i]), mode, sizeof(graphics_mode));
			graphics_modes[i].pNext = &(graphics_modes[i+1]);
		}
	}

	/* Hardcode the no graphics option */
	graphics_modes[count].pNext = NULL;
	graphics_modes[count].grafID = GRAPHICS_NONE;
	graphics_modes[count].alphablend = 0;
	graphics_modes[count].overdrawRow = 0;
	graphics_modes[count].overdrawMax = 0;
	my_strcpy(graphics_modes[count].pref, "none", 8);
	my_strcpy(graphics_modes[count].path, "", 32);
	my_strcpy(graphics_modes[count].file, "", 32);
	my_strcpy(graphics_modes[count].menuname, "None", 32);

	graphics_mode_high_id = max;

	/* Set the default graphics mode to be no graphics */
	current_graphics_mode = &(graphics_modes[count]);

	if (p) {
		mode = parser_priv(p);
		while (mode) {
			n = mode->pNext;
			mem_free(mode);
			mode = n;
		}
	
		parser_setpriv(p, NULL);
		parser_destroy(p);
	}
	return PARSE_ERROR_NONE;
}

static void print_error(const char *name, struct parser *p) {
	struct parser_state s;
	parser_getstate(p, &s);
	msg("Parse error in %s line %d column %d: %s: %s", name,
	           s.line, s.col, s.msg, parser_error_str[s.error]);
	event_signal(EVENT_MESSAGE_FLUSH);
}

bool init_graphics_modes(void) {
	char buf[1024];

	ang_file *f;
	struct parser *p;
	errr e = 0;

	/* Build the filename */
	path_build(buf, sizeof(buf), ANGBAND_DIR_TILES, "list.txt");

	f = file_open(buf, MODE_READ, FTYPE_TEXT);
	if (!f) {
		msg("Cannot open '%s'.", buf);
		finish_parse_grafmode(NULL);
	} else {
		char line[1024];

		p = init_parse_grafmode();
		while (file_getl(f, line, sizeof line)) {
			e = parser_parse(p, line);
			if (e != PARSE_ERROR_NONE) {
				print_error(buf, p);
				break;
			}
		}

		finish_parse_grafmode(p);
		file_close(f);
	}

	/* Result */
	return e == PARSE_ERROR_NONE;
}

void close_graphics_modes(void) {
	if (graphics_modes) {
		mem_free(graphics_modes);
		graphics_modes = NULL;
	}
}

graphics_mode *get_graphics_mode(uint8_t id) {
	graphics_mode *test = graphics_modes;
	while (test) {
		if (test->grafID == id) {
			return test;
		}
		test = test->pNext;
	}
	return NULL;
}

/**
 * Test for whether an attribute/character pair corresponds to a double-height
 * tile.
 * \param a Is the attribute.
 * \param c Is the character.
 * Intended for use as struct term's dblh_hook field.
 */
int is_dh_tile(int a, wchar_t c)
{
	int tileset_row;

	/*
	 * If it's not a tile (assumes tiles have high-bit set on the
	 * attribute), graphics aren't enabled, or the graphics mode doesn't
	 * use double-height tiles, it can't be double-height.
	 */
	if (!(a & 0x80) || !current_graphics_mode ||
			!current_graphics_mode->overdrawRow) {
		return 0;
	}
	/* Test the row for the tile. */
	tileset_row = a & 0x7f;
	return tileset_row >= current_graphics_mode->overdrawRow &&
		tileset_row <= current_graphics_mode->overdrawMax;
}
