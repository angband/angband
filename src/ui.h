/*
 * Copyright (c) 2007 Pete Mack and others
 * This code released under the Gnu Public License. See www.fsf.org
 * for current GPL license details. Addition permission granted to
 * incorporate modifications in all Angband variants as defined in the
 * Angband variants FAQ. See rec.games.roguelike.angband for FAQ.
 */



#ifndef INCLUDED_UI_H
#define INCLUDED_UI_H

/* ================== GEOMETRY ====================== */

/* Defines a rectangle on the screen that is bound to a Panel or subpanel */
typedef struct region region;

struct region {
	int col;	/* x-coordinate of upper right corner */
	int row;	/* y-coord of upper right coordinate */
	int width;	/* width of display area. 1 - use system default. */
			/* non-positive - rel to right of screen */
	int page_rows;	/* non-positive value is relative to the bottom of the screen */
};

/* Region that defines the full screen */
static const region SCREEN_REGION = {0, 0, 0, 0};

/* Erase the contents of a region */
void region_erase(const region *loc);

/* Erase the contents of a region + 1 char each way */
void region_erase_bordered(const region *loc);

/* Given a region with relative values, make them absolute */
region region_calculate(region loc);

/* Check whether a (mouse) event is inside a region */
bool region_inside(const region *loc, const ui_event *key);


/*** Text ***/

#include "z-textblock.h"
void textui_textblock_show(textblock *tb, region orig_area, const char *header);
void textui_textblock_place(textblock *tb, region orig_area, const char *header);


/*** Misc ***/

void window_make(int origin_x, int origin_y, int end_x, int end_y);


#endif /* INCLUDED_UI_H */
