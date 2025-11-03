/**
 * \file win-term.h
 * \brief Windows terminal structure.
 *
 * Copyright (c) 1997 Ben Harrison, Skirmantas Kligys, Robert Ruehlmann,
 * and others
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

#ifndef INCLUDED_WIN_TERM_H
#define INCLUDED_WIN_TERM_H

/**
 * Initial framework (and most code) by Ben Harrison (benh@phial.com).
 *
 * Original code by Skirmantas Kligys (kligys@scf.usc.edu).
 *
 * Additional code by Ross E Becker (beckerr@cis.ohio-state.edu),
 * and Chris R. Martin (crm7479@tam2000.tamu.edu).
 *
 * Additional code by Robert Ruehlmann <rr9@thangorodrim.net>.
 */


/**
 * Forward declare
 */
typedef struct _term_data term_data;

/**
 * Extra "term" data
 *
 * Note the use of "font_want" for the names of the font file requested by
 * the user, and the use of "font_file" for the currently active font file.
 *
 * The "font_file" is uppercased, and takes the form "8X13.FON", while
 * "font_want" can be in almost any form as long as it could be construed
 * as attempting to represent the name of a font.
 */
struct _term_data
{
	term t;

	const char *s;

	HWND w;

	DWORD dwStyle;
	DWORD dwExStyle;

	uint keys;

	uint16_t rows;
	uint16_t cols;

	uint pos_x;
	uint pos_y;
	uint size_wid;
	uint size_hgt;
	uint size_ow1;
	uint size_oh1;
	uint size_ow2;
	uint size_oh2;

	bool size_hack;

	bool xtra_hack;

	bool visible;
	bool maximized;

	bool bizarre;

	char *font_want;
	char *font_file;

	HFONT font_id;

	uint font_wid;
	uint font_hgt;

	uint tile_wid;
	uint tile_hgt;

	uint map_tile_wid;
	uint map_tile_hgt;

	bool map_active;
};


/**
 * Maximum number of windows XXX XXX XXX
 */
#define MAX_TERM_DATA 8

/* From win-layout.c */
int default_layout_win(term_data *data, int maxterms);

#endif /* INCLUDED_WIN_TERM_H */
