/**
 * \file ui-output.h
 * \brief Putting text on the screen, screen saving and loading, panel handling
 *
 * Copyright (c) 2007 Pete Mack and others.
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

#ifndef INCLUDED_UI_OUTPUT_H
#define INCLUDED_UI_OUTPUT_H

#include "ui-event.h"
#include "ui-term.h"
#include "z-textblock.h"

/**
 * ------------------------------------------------------------------------
 * Regions
 * ------------------------------------------------------------------------ */
/**
 * Defines a rectangle on the screen that is bound to a Panel or subpanel
 */
typedef struct region region;

struct region {
	int col;	/* x-coordinate of upper right corner */
	int row;	/* y-coord of upper right coordinate */
	int width;	/* width of display area. 1 - use system default. */
				/* non-positive - rel to right of screen */
	int page_rows;	/* non-positive value is relative to the bottom of the screen */
};

/**
 * Region that defines the full screen
 */
static const region SCREEN_REGION = {0, 0, 0, 0};

/**
 * Erase the contents of a region
 */
void region_erase(const region *loc);

/**
 * Erase the contents of a region + 1 char each way
 */
void region_erase_bordered(const region *loc);

/**
 * Given a region with relative values, make them absolute
 */
region region_calculate(region loc);

/**
 * Check whether a (mouse) event is inside a region
 */
bool region_inside(const region *loc, const ui_event *key);


/**
 * ------------------------------------------------------------------------
 * Text display
 * ------------------------------------------------------------------------ */

struct keypress textui_textblock_show(textblock *tb, region orig_area, const char *header);
void textui_textblock_place(textblock *tb, region orig_area, const char *header);

/**
 * ------------------------------------------------------------------------
 * text_out hook for screen display
 * ------------------------------------------------------------------------ */
void text_out_to_screen(uint8_t a, const char *str);

/**
 * ------------------------------------------------------------------------
 * Simple text display
 * ------------------------------------------------------------------------ */
void c_put_str(uint8_t attr, const char *str, int row, int col);
void put_str(const char *str, int row, int col);
void c_prt(uint8_t attr, const char *str, int row, int col);
void prt(const char *str, int row, int col);

/**
 * ------------------------------------------------------------------------
 * Screen loading/saving
 * ------------------------------------------------------------------------ */
extern int16_t screen_save_depth;
void screen_save(void);
void screen_load(void);
bool textui_map_is_visible(void);


/**
 * ------------------------------------------------------------------------
 * Miscellaneous things
 * ------------------------------------------------------------------------ */
void window_make(int origin_x, int origin_y, int end_x, int end_y);
bool panel_should_modify(term *t, int wy, int wx);
bool modify_panel(term *t, int wy, int wx);
bool change_panel(int dir);
void verify_panel(void);
void center_panel(void);
void textui_get_panel(int *min_y, int *min_x, int *max_y, int *max_x);
bool textui_panel_contains(unsigned int y, unsigned int x);

#endif /* INCLUDED_UI_OUTPUT_H */
