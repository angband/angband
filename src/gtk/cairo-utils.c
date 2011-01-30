/*
 * File: cairo_utils.c
 * Purpose: Cairo calls for use in Angband ports
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

#ifdef USE_GTK
#include "cairo-utils.h"

#define USE_PANGO
void set_foreground_color(cairo_t *cr, byte a)
{
	cairo_set_source_rgb(cr, 
	(double)angband_color_table[a][1] / 256, 
	(double)angband_color_table[a][2] / 256, 
	(double)angband_color_table[a][3] / 256);
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
	if (cr !=NULL)
		cairo_rectangle (cr, r.x, r.y, r.width, r.height);
}

/*
 * Erase the whole term.
 */
void cairo_clear(cairo_surface_t *surface, cairo_rectangle_t r, byte c)
{
	cairo_t *cr = NULL;
	
	if ((surface != NULL) &&(cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS))
	{
	cr = cairo_create(surface);
	
	if (cr !=NULL)
	{
		cairo_save(cr);
		c_rect(cr, r);
		set_foreground_color(cr, c);
		cairo_fill(cr);
		cairo_close_path(cr);
		cairo_restore(cr);
	}
	
		cairo_destroy(cr);
	}
}

void cairo_cursor(cairo_surface_t *surface, cairo_rectangle_t r, byte c)
{
	cairo_t *cr;
	
	if (surface != NULL)
	{
	cr = cairo_create(surface);
	if (cr !=NULL)
	{
		cairo_save(cr);
		c_rect(cr, r);
		
		set_foreground_color(cr, c);
	
		cairo_set_operator(cr, CAIRO_OPERATOR_ADD);
		cairo_fill(cr);
		cairo_close_path(cr);
		cairo_restore(cr);
	}
		cairo_destroy(cr);
	}
}

void drawn_progress_bar(cairo_surface_t *surface, font_info *font, int x, int y, float curr, float max, byte color, int len)
{
	cairo_t *cr;
	int temp;
	cairo_rectangle_t r;
	float percent;
	measurements size;
	
	 size.w = font->w;
	size.h = font->h;
	
	if (surface != NULL)
	{
	cr = cairo_create(surface);
	
	if (max > 0)
		percent = curr / max;
	else
		percent = 0;
	
	init_cairo_rect(&r, (size.w * x)+ 1, (size.h) * y + 1,  (size.w * len) - 1, size.h - 2);
	cairo_clear(surface, r, TERM_DARK);
	
	temp = cairo_get_line_width(cr);
	set_foreground_color(cr, color);
	
	if (percent > 0)
	{
		cairo_set_line_width(cr, size.h);
		cairo_move_to(cr, r.x, r.y + (r.height * 0.5));
		cairo_line_to(cr, (size.w * ((len * percent) + x)), r.y + (r.height * 0.5));
		cairo_stroke(cr);
	}
	if (max > 0)
	{
		cairo_set_line_width(cr, 1);
		
		set_foreground_color(cr, TERM_SLATE);
		c_rect(cr, r);
		cairo_stroke(cr);
		
		cairo_set_line_width(cr, temp);
	}
		cairo_destroy(cr);
	}
}

void draw_tile(cairo_t *cr, cairo_matrix_t m, cairo_rectangle_t r, int tx, int ty)
{
	if (cr !=NULL)
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
}

cairo_matrix_t cairo_font_scaling(cairo_surface_t *surface, double tile_w, double tile_h, double font_w, double font_h)
{
	cairo_t *cr;
	cairo_matrix_t m;
	double sx, sy;

	if (surface != NULL)
	{
	cr = cairo_create(surface);
	if (cr !=NULL)
	{
		/* Get a matrix set up to scale the graphics. */
		cairo_get_matrix(cr, &m);
		sx = (tile_w)/(font_w);
		sy = (tile_h)/(font_h);
		cairo_matrix_scale(&m, sx, sy);
	}
	
	cairo_destroy(cr);
	}
	return(m);
}

void cairo_draw_from_surface(cairo_t *cr, cairo_surface_t *surface, cairo_rectangle_t r)
{	
	if ((cr !=NULL) && (surface != NULL))
	{
		cairo_save(cr);
		c_rect(cr, r);
		cairo_set_source_surface(cr, surface, 0, 0);
		cairo_fill(cr);
		cairo_restore(cr);
	}
}

/*
 * Main reason for the lengthy header is strictly so we don't rely on the font size and tile size
 * having one particular naming convention in term data. That should probably be standardized
 * across the board, honestly. 
 */
void draw_tiles(
cairo_surface_t *surface, int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp, 
font_info *font, measurements *actual, measurements *tile)
{
	cairo_t *cr;
	cairo_rectangle_t char_rect, r;
	int i;
	
	/* Tile & Current Position */
	int tx, ty;
	int cx, cy;
	
	if (surface != NULL)
	{
	cr = cairo_create(surface);
	if (cr !=NULL)
	{
	init_cairo_rect(&r, x * font->w, y * font->h,  actual->w * n, actual->h);
	
	cairo_clear(surface, r, TERM_DARK);
	
	/* Get the current position, Minus cx, which changes for each iteration */
	cx = 0;
	cy = (y * font->h);
	
	for (i = 0; i < n; i++)
	{
		/* Increment x 1 step; use the font width because of equippy chars and the gap between 
   		 * the status bar and the map.
		 */
		cx += x * font->w;
		init_cairo_rect(&char_rect, cx, cy, actual->w, actual->h);
		
		cairo_clear(surface, char_rect, TERM_DARK);
		/* Get the terrain tile, scaled to the font size */
		tx= (tcp[i] & 0x7F) * actual->w;
		ty = (tap[i] & 0x7F) * actual->h;
		
		draw_tile(cr, matrix, char_rect, tx, ty);
	
		/* If foreground is the same as background, we're done */
		if ((tap[i] == ap[i]) && (tcp[i] == cp[i])) continue;
		
		/* Get the foreground tile size, scaled to the font size */
		tx = (cp[i] & 0x7F) * actual->w;
		ty = (ap[i] & 0x7F) * actual->h;
	
		draw_tile(cr, matrix, char_rect, tx, ty);
	}
	}
	cairo_destroy(cr);
	}
}

void get_font_size(font_info *font)
{
	#ifndef USE_PANGO
	get_toy_font_size(font);
	#else
	PangoRectangle r;
	PangoLayout *temp;
	PangoFontDescription *temp_font;
	
	cairo_t *cr;
	cairo_surface_t *surface;
	
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 200, 200);
	cr = cairo_create(surface);
	
	temp = pango_cairo_create_layout(cr);
	temp_font = pango_font_description_from_string(font->name);
	
	/* Draw an @, and measure it */
	pango_layout_set_font_description(temp, temp_font);
	pango_layout_set_text(temp, "@", 1);
	pango_cairo_show_layout(cr, temp);
	pango_layout_get_pixel_extents(temp, NULL, &r);
	
	font->w = r.width;
	font->h = r.height;
	
	pango_font_description_free(temp_font);
	cairo_destroy(cr);
	cairo_surface_destroy(surface);
	g_object_unref(temp);
	#endif
}

void draw_text(cairo_surface_t *surface, font_info *font, measurements *actual, int x, int y, int n, byte a, const char *s)
{
	cairo_t *cr;
	
	if (surface != NULL)
	{
	cairo_rectangle_t r;
	PangoLayout *layout;
	PangoFontDescription *temp_font;
		
	cr = cairo_create(surface);
	#ifndef USE_PANGO
	draw_toy_text(cr, font, actual, x, y, n, a, s);
	#else
	
	if (cr !=NULL)
	{
	init_cairo_rect(&r, x * font->w, y * font->h,  actual->w * n, actual->h);
	
	cairo_clear(surface, r, TERM_DARK);
		
	/* Create a PangoLayout, set the font and text */
	layout = pango_cairo_create_layout(cr); 
	
	temp_font = pango_font_description_from_string(font->name);
	set_foreground_color(cr, a);
	pango_layout_set_text(layout, s, n);
	pango_layout_set_font_description(layout, temp_font);
	
	/* Draw the text to the pixmap */
	cairo_move_to(cr, x * font->w, y * font->h);
	
	pango_cairo_show_layout(cr, layout);
	g_object_unref(G_OBJECT(layout));
	}
	#endif
	cairo_destroy(cr);
	}
}

/* Experimental - Currently messes up the display if larger then 12 point Monospace */
void set_cairo_font_size(cairo_t *cr, font_info *font)
{
	double size;
	cairo_font_extents_t extents;
	cairo_text_extents_t text_extents;
	
	cairo_select_font_face(cr, font->family, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	size = font->size * (96.0/72.0);
	
	cairo_set_font_size(cr, size);
	
	cairo_font_extents(cr, &extents);
	cairo_text_extents(cr, "@", &text_extents);
	
	font->w = extents.max_x_advance;
	font->h = extents.height;
	font->descent = extents.descent;
	
}

void get_toy_font_size(font_info *font)
{
	cairo_t *cr;
	cairo_surface_t *surface;
	
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 200, 200);
	cr = cairo_create(surface);
	
	set_cairo_font_size(cr, font);
	
	cairo_destroy(cr);
	cairo_surface_destroy(surface);
}
void draw_toy_text(cairo_t *cr, font_info *font, measurements *actual, int x, int y, int n, byte a, const char *s)
{
	char str[255];
	cairo_rectangle_t r;
	
	set_cairo_font_size(cr, font);
	
	init_cairo_rect(&r, x * font->w, y * font->h,  font->w * n, font->h);
	
	c_rect(cr,r);
	cairo_stroke(cr);
	
	my_strcpy(str, s, n + 1);
	
	if (cr !=NULL)
	{
		set_foreground_color(cr, a);
		cairo_move_to(cr, x * (font->w), (y + 1) * font->h - font->descent);
		cairo_show_text(cr, str);
	}
}

#endif  /*USE_GTK */
