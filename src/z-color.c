/*
 * File: z-color.c
 * Purpose: generic color definitions
 *
 * Copyright (c) 1997 Ben Harrison
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
#include "h-basic.h"
#include "z-color.h"
#include "z-util.h"

/*** Colour constants ***/

/*
 * Global table of color definitions (mostly zeros)
 */
byte angband_color_table[MAX_COLORS][4] =
{
	{0x00, 0x00, 0x00, 0x00}, /* 0  TERM_DARK */
	{0x00, 0xff, 0xff, 0xff}, /* 1  TERM_WHITE */
	{0x00, 0x80, 0x80, 0x80}, /* 2  TERM_SLATE */
	{0x00, 0xff, 0x80, 0x00}, /* 3  TERM_ORANGE */
	{0x00, 0xc0, 0x00, 0x00}, /* 4  TERM_RED */
	{0x00, 0x00, 0x80, 0x40}, /* 5  TERM_GREEN */
	{0x00, 0x00, 0x40, 0xff}, /* 6  TERM_BLUE */
	{0x00, 0x80, 0x40, 0x00}, /* 7  TERM_UMBER */
	{0x00, 0x60, 0x60, 0x60}, /* 8  TERM_L_DARK */
	{0x00, 0xc0, 0xc0, 0xc0}, /* 9  TERM_L_WHITE */
	{0x00, 0xff, 0x00, 0xff}, /* 10 TERM_L_PURPLE */
	{0x00, 0xff, 0xff, 0x00}, /* 11 TERM_YELLOW */
	{0x00, 0xff, 0x40, 0x40}, /* 12 TERM_L_RED */
	{0x00, 0x00, 0xff, 0x00}, /* 13 TERM_L_GREEN */
	{0x00, 0x00, 0xff, 0xff}, /* 14 TERM_L_BLUE */
	{0x00, 0xc0, 0x80, 0x40}, /* 15 TERM_L_UMBER */
	{0x00, 0x90, 0x00, 0x90}, /* 16 TERM_PURPLE */
	{0x00, 0x90, 0x20, 0xff}, /* 17 TERM_VIOLET */
	{0x00, 0x00, 0xa0, 0xa0}, /* 18 TERM_TEAL */
	{0x00, 0x6c, 0x6c, 0x30}, /* 19 TERM_MUD */
	{0x00, 0xff, 0xff, 0x90}, /* 20 TERM_L_YELLOW */
	{0x00, 0xff, 0x00, 0xa0}, /* 21 TERM_MAGENTA */
	{0x00, 0x20, 0xff, 0xdc}, /* 22 TERM_L_TEAL */
	{0x00, 0xb8, 0xa8, 0xff}, /* 23 TERM_L_VIOLET */
	{0x00, 0xff, 0x80, 0x80}, /* 24 TERM_L_PINK */
	{0x00, 0xb4, 0xb4, 0x00}, /* 25 TERM_MUSTARD */
	{0x00, 0xa0, 0xc0, 0xd0}, /* 26 TERM_BLUE_SLATE */
	{0x00, 0x00, 0xb0, 0xff}, /* 27 TERM_DEEP_L_BLUE */
	{0x00, 0x28, 0x28, 0x28}, /* 28 TERM_SHADE */
};

/*
 * Global array of color names and translations.
 */
color_type color_table[MAX_COLORS] =
{
	/* full mono vga blind lighter darker highlight metallic misc */
	{'d', "Dark", {0, 0, 0, TERM_DARK, TERM_L_DARK, TERM_DARK,
				   TERM_L_DARK, TERM_L_DARK, TERM_DARK}},

	{'w', "White", {1, 1, 1, TERM_WHITE, TERM_YELLOW, TERM_SLATE,
					TERM_L_BLUE, TERM_YELLOW, TERM_WHITE}},

	{'s', "Slate", {2, 1, 2, TERM_SLATE, TERM_L_WHITE, TERM_L_DARK,
					TERM_L_WHITE, TERM_L_WHITE, TERM_SLATE}},

	{'o', "Orange", {3, 1, 3, TERM_L_WHITE, TERM_YELLOW, TERM_SLATE,
					 TERM_YELLOW, TERM_YELLOW, TERM_ORANGE}},

	{'r', "Red", {4, 1, 4, TERM_SLATE, TERM_L_RED, TERM_SLATE,
				  TERM_L_RED, TERM_L_RED, TERM_RED}},

	{'g', "Green", {5, 1, 5, TERM_SLATE, TERM_L_GREEN, TERM_SLATE,
					TERM_L_GREEN, TERM_L_GREEN, TERM_GREEN}},

	{'b', "Blue", {6, 1, 6, TERM_SLATE, TERM_L_BLUE, TERM_SLATE,
				   TERM_L_BLUE, TERM_L_BLUE, TERM_BLUE}},

	{'u', "Umber", {7, 1, 7, TERM_L_DARK, TERM_L_UMBER, TERM_L_DARK,
					TERM_L_UMBER, TERM_L_UMBER, TERM_UMBER}},

	{'D', "Light Dark", {8, 1, 8, TERM_L_DARK, TERM_SLATE, TERM_L_DARK,
						 TERM_SLATE, TERM_SLATE, TERM_L_DARK}},

	{'W', "Light Slate", {9, 1, 9, TERM_L_WHITE, TERM_WHITE, TERM_SLATE,
						  TERM_WHITE, TERM_WHITE, TERM_SLATE}},

	{'P', "Light Purple", {10, 1, 10, TERM_SLATE, TERM_YELLOW, TERM_SLATE,
						   TERM_YELLOW, TERM_YELLOW, TERM_L_PURPLE}},

	{'y', "Yellow", {11, 1, 11, TERM_L_WHITE, TERM_L_YELLOW, TERM_L_WHITE,
					 TERM_WHITE, TERM_WHITE, TERM_YELLOW}},

	{'R', "Light Red", {12, 1, 12, TERM_L_WHITE, TERM_YELLOW, TERM_RED,
						TERM_YELLOW, TERM_YELLOW, TERM_L_RED}},

	{'G', "Light Green", {13, 1, 13, TERM_L_WHITE, TERM_YELLOW, TERM_GREEN,
						  TERM_YELLOW, TERM_YELLOW, TERM_L_GREEN}},

	{'B', "Light Blue", {14, 1, 14, TERM_L_WHITE, TERM_YELLOW, TERM_BLUE,
						 TERM_YELLOW, TERM_YELLOW, TERM_L_BLUE}},

	{'U', "Light Umber", {15, 1, 15, TERM_L_WHITE, TERM_YELLOW, TERM_UMBER,
						  TERM_YELLOW, TERM_YELLOW, TERM_L_UMBER}},

	/* "new" colors */
	{'p', "Purple", {16, 1, 10,TERM_SLATE, TERM_L_PURPLE, TERM_SLATE,
					 TERM_L_PURPLE, TERM_L_PURPLE, TERM_L_PURPLE}},

	{'v', "Violet", {17, 1, 10,TERM_SLATE, TERM_L_PURPLE, TERM_SLATE,
					 TERM_L_PURPLE, TERM_L_PURPLE, TERM_L_PURPLE}},

	{'t', "Teal", {18, 1, 6, TERM_SLATE, TERM_L_TEAL, TERM_SLATE,
				   TERM_L_TEAL, TERM_L_TEAL, TERM_L_BLUE}},

	{'m', "Mud", {19, 1, 5, TERM_SLATE, TERM_MUSTARD, TERM_SLATE,
				  TERM_MUSTARD, TERM_MUSTARD, TERM_UMBER}},

	{'Y', "Light Yellow", {20, 1, 11, TERM_WHITE, TERM_WHITE, TERM_YELLOW,
						   TERM_WHITE, TERM_WHITE, TERM_L_YELLOW}},

	{'i', "Magenta-Pink", {21, 1, 12, TERM_SLATE, TERM_L_PINK, TERM_RED,
						   TERM_L_PINK, TERM_L_PINK, TERM_L_PURPLE}},

	{'T', "Light Teal", {22, 1, 14, TERM_L_WHITE, TERM_YELLOW, TERM_TEAL,
						 TERM_YELLOW, TERM_YELLOW, TERM_L_BLUE}},

	{'V', "Light Violet", {23, 1, 10, TERM_L_WHITE, TERM_YELLOW, TERM_VIOLET,
						   TERM_YELLOW, TERM_YELLOW, TERM_L_PURPLE}},

	{'I', "Light Pink", {24, 1, 12, TERM_L_WHITE, TERM_YELLOW, TERM_MAGENTA,
						 TERM_YELLOW, TERM_YELLOW, TERM_L_PURPLE}},

	{'M', "Mustard", {25, 1, 11, TERM_SLATE, TERM_YELLOW, TERM_SLATE,
					  TERM_YELLOW, TERM_YELLOW, TERM_YELLOW}},

	{'z', "Blue Slate",  {26, 1, 9, TERM_SLATE, TERM_DEEP_L_BLUE, TERM_SLATE,
						  TERM_DEEP_L_BLUE, TERM_DEEP_L_BLUE, TERM_L_WHITE}},

	{'Z', "Deep Light Blue", {27, 1, 14, TERM_L_WHITE, TERM_L_BLUE, TERM_BLUE_SLATE,
							  TERM_L_BLUE, TERM_L_BLUE, TERM_L_BLUE}},

	/* Rest to be filled in when the game loads */
};



/*
 * Accept a color index character; if legal, return the color.  -LM-
 *
 * Unlike Sangband, we don't translate these colours here.
 */
/* XXX: having color_{char,text}_to_attr() separately is moronic. */
int color_char_to_attr(char c)
{
	int a;

	/* Is negative -- spit it right back out */
	if (c < 0) return (c);

	/* Is a space or '\0' -- return black */
	if (c == '\0' || c == ' ') return (TERM_DARK);

	/* Search the color table */
	for (a = 0; a < BASIC_COLORS; a++)
	{
		/* Look for the index */
		if (color_table[a].index_char == c) break;
	}

	/* If we don't find the color, we assume white */
	if (a == BASIC_COLORS) return (TERM_WHITE);

	/* Return the color */
	return (a);
}


/*
 * Converts a string to a terminal color byte.
 */
int color_text_to_attr(const char *name)
{
	int a;

	for (a = 0; a < MAX_COLORS; a++)
	{
		if (my_stricmp(name, color_table[a].name) == 0) return (a);
	}

	/* Default to white */
	return (TERM_WHITE);
}


/*
 * Extract a textual representation of an attribute
 */
const char *attr_to_text(byte a)
{
	if (a < BASIC_COLORS)
		return (color_table[a].name);
	else
		return ("Icky");
}
