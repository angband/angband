/*
 * File: main-cairo.c
 * Purpose: Cairo calls for use in Angband ports
 * (Currently for the Gtk port, but should be reusable.)
 *
 * Copyright (c) 2007 Shanoah Alkire
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

#ifdef USE_GTK

#include "cairo-utils.h"

void set_foreground_color(cairo_t *cr, byte a)
{
	double red, green, blue;

	red   = angband_color_table[a][1] * 256;
	green = angband_color_table[a][2] * 256;
	blue  = angband_color_table[a][3] * 256;
	
	cairo_set_source_rgb(cr, red / 65536, green / 65536, blue / 65536);
}

const char * colour_as_text(byte a)
{
	unsigned int red, green, blue;
	char* c, str[10] = "#FFFFFF";

	c = str;
	red   = angband_color_table[a][1];
	green = angband_color_table[a][2];
	blue  = angband_color_table[a][3];
	
	sprintf(str, "#%x%x%x", red, green, blue);
	plog_fmt("Color: %s; %s", str);
	return(c);
}

void init_cairo_rect(cairo_rectangle_t *r, int x, int y, int w, int h)
{
	r->x = x;
	r->y = y;
	r->width = w;
	r->height = h;
}

void c_rect(cairo_t *cr, cairo_rectangle_t r)
{
	cairo_rectangle (cr, r.x, r.y, r.width, r.height);
}

/*
 * Erase the whole term.
 */
void cairo_clear(cairo_t *cr, cairo_rectangle_t r, byte c)
{
	cairo_save(cr);
	c_rect(cr, r);
	set_foreground_color(cr, c);
	cairo_fill(cr);
	cairo_close_path(cr);
	cairo_restore(cr);
}

void cairo_cursor(cairo_t *cr, cairo_rectangle_t r, byte c)
{
	cairo_save(cr);
	c_rect(cr, r);
	set_foreground_color(cr, c);
	
	cairo_set_operator(cr, CAIRO_OPERATOR_ADD);
	cairo_fill(cr);
	cairo_close_path(cr);
	cairo_restore(cr);
}

void draw_tile(cairo_t *cr, cairo_matrix_t m, cairo_rectangle_t r, int tx, int ty)
{
	
	cairo_save(cr); 
	
	/* Use the rect and pattern */
	c_rect(cr, r);
	cairo_set_source (cr, tile_pattern);
	
	/* Pull the tile we need */
	cairo_surface_set_device_offset(graphical_tiles, tx - r.x, ty - r.y);
	
	/* Use transparency */
	cairo_set_operator(cr, CAIRO_OPERATOR_ADD);
	
	/* Use the matrix with our pattern */
	cairo_pattern_set_matrix(tile_pattern, &m);
	
	/* Draw it */
	cairo_fill(cr);
	
	cairo_restore(cr);
}

cairo_matrix_t cairo_font_scaling(cairo_t *cr, double tile_w, double tile_h, double font_w, double font_h)
{
	cairo_matrix_t m;
	double sx, sy;

	/* Get a matrix set up to scale the graphics. */
	cairo_get_matrix(cr, &m);
	sx = (tile_w)/(font_w);
	sy = (tile_h)/(font_h);
	
	cairo_matrix_scale(&m, sx, sy);
	return(m);
}

void cairo_draw_from_surface(cairo_t *cr, cairo_surface_t *surface, cairo_rectangle_t r)
{
	cairo_save(cr);
	c_rect(cr, r);
	cairo_set_source_surface(cr, surface, 0, 0);
	cairo_fill(cr);
	cairo_restore(cr);
}

/*
 * Main reason for the lengthy header is strictly so we don't rely on the font size and tile size
 * having one particular naming convention in term data. That should probably be standardized
 * across the board, honestly. 
 */
void draw_tiles(
cairo_t *cr, int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp, 
int font_h, int font_w, int tile_h, int tile_w)
{
	cairo_rectangle_t char_rect;
	cairo_matrix_t m;
	int i;
	
	/* Tile & Current Position */
	int tx, ty;
	int cx, cy;
	
	/* Get a matrix set up to scale the graphics. */
	m = cairo_font_scaling(cr, tile_w, tile_h, font_w, font_h);
	
	/* Get the current position, Minus cx, which changes for each iteration */
	cx = 0;
	cy = (y * font_h);
	
	for (i = 0; i < n; i++)
	{
		/* Increment x 1 step */
		cx += (x* font_w);
		init_cairo_rect(&char_rect, cx, cy, font_w, font_h);
		
		/* Get the terrain tile, scaled to the font size */
		tx= (tcp[i] & 0x7F) * font_w;
		ty = (tap[i] & 0x7F) * font_h;
		
		draw_tile(cr, m, char_rect, tx, ty);
	
		/* If foreground is the same as background, we're done */
		if ((tap[i] == ap[i]) && (tcp[i] == cp[i])) continue;
		
		/* Get the foreground tile size, scaled to the font size */
		tx = (cp[i] & 0x7F) * font_w;
		ty = (ap[i] & 0x7F) * font_h;
	
		draw_tile(cr, m, char_rect, tx, ty);
	}
}
#endif
