/**
 * \file ui-target.c
 * \brief UI for targetting code
 *
 * Copyright (c) 1997-2014 Angband contributors
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
#include "ui-target.h"

/**
 * Display targeting help at the bottom of the screen.
 */
void target_display_help(bool monster, bool free)
{
	/* Determine help location */
	int wid, hgt, help_loc;
	Term_get_size(&wid, &hgt);
	help_loc = hgt - HELP_HEIGHT;
	
	/* Clear */
	clear_from(help_loc);

	/* Prepare help hooks */
	text_out_hook = text_out_to_screen;
	text_out_indent = 1;
	Term_gotoxy(1, help_loc);

	/* Display help */
	text_out_c(TERM_L_GREEN, "<dir>");
	text_out(" and ");
	text_out_c(TERM_L_GREEN, "<click>");
	text_out(" look around. '");
	text_out_c(TERM_L_GREEN, "g");
	text_out(" moves to the selection. '");
	text_out_c(TERM_L_GREEN, "p");
	text_out("' selects the player. '");
	text_out_c(TERM_L_GREEN, "q");
	text_out("' exits. '");
	text_out_c(TERM_L_GREEN, "r");
	text_out("' displays details. '");

	if (free)
	{
		text_out_c(TERM_L_GREEN, "m");
		text_out("' restricts to interesting places. ");
	}
	else
	{
		text_out_c(TERM_L_GREEN, "+");
		text_out("' and '");
		text_out_c(TERM_L_GREEN, "-");
		text_out("' cycle through interesting places. '");
		text_out_c(TERM_L_GREEN, "o");
		text_out("' allows free selection. ");
	}
	
	if (monster || free)
	{
		text_out("'");
		text_out_c(TERM_L_GREEN, "t");
		text_out("' targets the current selection.");
	}

	/* Reset */
	text_out_indent = 0;
}

