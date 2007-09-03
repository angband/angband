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
#ifndef INCLUDED_CAIRO_UTILS_H
#define INCLUDED_CAIRO_UTILS_H

#include "cairo.h"
 
/* Set the current color */
extern void set_foreground_color(cairo_t *cr, byte a);
extern const char *colour_as_text(byte a);
/* Set up a GdkRectangle easily */
/*void init_gdk_rect(GdkRectangle *r, int x, int y, int w, int h);*/
extern void init_cairo_rect(cairo_rectangle_t *r, int x, int y, int w, int h);

/* Use it as a cairo rectangle */
/*void c_rect(cairo_t *cr, GdkRectangle r);*/
extern void c_rect(cairo_t *cr, cairo_rectangle_t r);

extern cairo_matrix_t cairo_font_scaling(cairo_t *cr, double tile_w, double tile_h, double font_w, double font_h);
extern void cairo_clear(cairo_t *cr, cairo_rectangle_t r, byte c);
extern void cairo_cursor(cairo_t *cr, cairo_rectangle_t r, byte c);
extern void draw_tile(cairo_t *cr, cairo_matrix_t m, cairo_rectangle_t r, int tx, int ty);
extern void draw_tiles(cairo_t *cr, int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp, int font_h, int font_w, int tile_h, int tile_w);
void cairo_draw_from_surface(cairo_t *cr, cairo_surface_t *surface, cairo_rectangle_t r);

cairo_surface_t* graphical_tiles;
cairo_pattern_t *tile_pattern;

#endif /* INCLUDED_CAIRO_UTILS_H */
