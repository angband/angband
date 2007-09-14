/*
 * File: main-cairo.h
 * Purpose: Cairo calls header for use in Angband ports
 * (Currently for the Gtk port, but should be reusable.)
 *
 * Copyright (c) 2000-2007 Robert Ruehlmann, Shanoah Alkire
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

#ifndef INCLUDED_CAIRO_UTILS_H 
#define INCLUDED_CAIRO_UTILS_H

#include "cairo.h" 
#include <gtk/gtk.h>

typedef struct font_info font_info;
typedef struct point point;
typedef struct measurements measurements;
struct font_info
{
	char name[256];
	int w,h;
};

struct point
{
	int x,y;
};

struct measurements
{
	int w,h;
};
cairo_surface_t* graphical_tiles;
cairo_pattern_t *tile_pattern;

/* Set the current color */
extern void set_foreground_color(cairo_t *cr, byte a);

/* Set up a cairo rectangle easily */
extern void init_cairo_rect(cairo_rectangle_t *r, int x, int y, int w, int h);

/* Use it as a cairo rectangle */
extern void c_rect(cairo_t *cr, cairo_rectangle_t r);

extern cairo_matrix_t cairo_font_scaling(cairo_t *cr, double tile_w, double tile_h, double font_w, double font_h);
extern void cairo_clear(cairo_t *cr, cairo_rectangle_t r, byte c);
extern void cairo_cursor(cairo_t *cr, cairo_rectangle_t r, byte c);
extern void draw_tile(cairo_t *cr, cairo_matrix_t m, cairo_rectangle_t r, int tx, int ty);
extern void draw_tiles(
cairo_t *cr, int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp, 
font_info font, measurements tile);
extern void cairo_draw_from_surface(cairo_t *cr, cairo_surface_t *surface, cairo_rectangle_t r);
extern void init_cairo(cairo_t *cr, cairo_surface_t *surface, measurements size);
extern void get_font_size(font_info *font);
extern void draw_text(cairo_t *cr, font_info *font, int x, int y, int n, byte a, cptr s);
#endif /* INCLUDED_CAIRO_UTILS_H*/
