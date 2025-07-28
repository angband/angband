/**
 * \file z-color.c
 * \brief Generic color definitions
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

/**
 * ------------------------------------------------------------------------
 * Colour constants
 * ------------------------------------------------------------------------ */

/**
 * Global table of color definitions (mostly zeros)
 */
uint8_t angband_color_table[MAX_COLORS][4] =
{
	{0x00, 0x00, 0x00, 0x00}, /* 0  COLOUR_DARK */
	{0x00, 0xff, 0xff, 0xff}, /* 1  COLOUR_WHITE */
	{0x00, 0x80, 0x80, 0x80}, /* 2  COLOUR_SLATE */
	{0x00, 0xff, 0x80, 0x00}, /* 3  COLOUR_ORANGE */
	{0x00, 0xc0, 0x00, 0x00}, /* 4  COLOUR_RED */
	{0x00, 0x00, 0x80, 0x40}, /* 5  COLOUR_GREEN */
	{0x00, 0x00, 0x40, 0xff}, /* 6  COLOUR_BLUE */
	{0x00, 0x80, 0x40, 0x00}, /* 7  COLOUR_UMBER */
	{0x00, 0x60, 0x60, 0x60}, /* 8  COLOUR_L_DARK */
	{0x00, 0xc0, 0xc0, 0xc0}, /* 9  COLOUR_L_WHITE */
	{0x00, 0xff, 0x00, 0xff}, /* 10 COLOUR_L_PURPLE */
	{0x00, 0xff, 0xff, 0x00}, /* 11 COLOUR_YELLOW */
	{0x00, 0xff, 0x40, 0x40}, /* 12 COLOUR_L_RED */
	{0x00, 0x00, 0xff, 0x00}, /* 13 COLOUR_L_GREEN */
	{0x00, 0x00, 0xff, 0xff}, /* 14 COLOUR_L_BLUE */
	{0x00, 0xc0, 0x80, 0x40}, /* 15 COLOUR_L_UMBER */
	{0x00, 0x90, 0x00, 0x90}, /* 16 COLOUR_PURPLE */
	{0x00, 0x90, 0x20, 0xff}, /* 17 COLOUR_VIOLET */
	{0x00, 0x00, 0xa0, 0xa0}, /* 18 COLOUR_TEAL */
	{0x00, 0x6c, 0x6c, 0x30}, /* 19 COLOUR_MUD */
	{0x00, 0xff, 0xff, 0x90}, /* 20 COLOUR_L_YELLOW */
	{0x00, 0xff, 0x00, 0xa0}, /* 21 COLOUR_MAGENTA */
	{0x00, 0x20, 0xff, 0xdc}, /* 22 COLOUR_L_TEAL */
	{0x00, 0xb8, 0xa8, 0xff}, /* 23 COLOUR_L_VIOLET */
	{0x00, 0xff, 0x80, 0x80}, /* 24 COLOUR_L_PINK */
	{0x00, 0xb4, 0xb4, 0x00}, /* 25 COLOUR_MUSTARD */
	{0x00, 0xa0, 0xc0, 0xd0}, /* 26 COLOUR_BLUE_SLATE */
	{0x00, 0x00, 0xb0, 0xff}, /* 27 COLOUR_DEEP_L_BLUE */
	{0x00, 0x28, 0x28, 0x28}, /* 28 COLOUR_SHADE */
};

/**
 * Global array of color names and translations.
 */
color_type color_table[MAX_COLORS] =
{
	/* full mono vga blind lighter darker highlight metallic misc */
	{'d', "Dark", {0, 0, 0, COLOUR_DARK, COLOUR_L_DARK, COLOUR_DARK,
				   COLOUR_L_DARK, COLOUR_L_DARK, COLOUR_DARK}},

	{'w', "White", {1, 1, 1, COLOUR_WHITE, COLOUR_YELLOW, COLOUR_L_WHITE,
					COLOUR_L_BLUE, COLOUR_YELLOW, COLOUR_WHITE}},

	{'s', "Slate", {2, 1, 2, COLOUR_SLATE, COLOUR_L_WHITE, COLOUR_L_DARK,
					COLOUR_L_WHITE, COLOUR_L_WHITE, COLOUR_SLATE}},

	{'o', "Orange", {3, 1, 3, COLOUR_L_WHITE, COLOUR_YELLOW, COLOUR_SLATE,
					 COLOUR_YELLOW, COLOUR_YELLOW, COLOUR_ORANGE}},

	{'r', "Red", {4, 1, 4, COLOUR_SLATE, COLOUR_L_RED, COLOUR_SLATE,
				  COLOUR_L_RED, COLOUR_L_RED, COLOUR_RED}},

	{'g', "Green", {5, 1, 5, COLOUR_SLATE, COLOUR_L_GREEN, COLOUR_SLATE,
					COLOUR_L_GREEN, COLOUR_L_GREEN, COLOUR_GREEN}},

	{'b', "Blue", {6, 1, 6, COLOUR_SLATE, COLOUR_L_BLUE, COLOUR_SLATE,
				   COLOUR_L_BLUE, COLOUR_L_BLUE, COLOUR_BLUE}},

	{'u', "Umber", {7, 1, 7, COLOUR_L_DARK, COLOUR_L_UMBER, COLOUR_L_DARK,
					COLOUR_L_UMBER, COLOUR_L_UMBER, COLOUR_UMBER}},

	{'D', "Light Dark", {8, 1, 8, COLOUR_L_DARK, COLOUR_SLATE, COLOUR_L_DARK,
						 COLOUR_SLATE, COLOUR_SLATE, COLOUR_L_DARK}},

	{'W', "Light Slate", {9, 1, 9, COLOUR_L_WHITE, COLOUR_WHITE, COLOUR_SLATE,
						  COLOUR_WHITE, COLOUR_WHITE, COLOUR_SLATE}},

	{'P', "Light Purple", {10, 1, 10, COLOUR_SLATE, COLOUR_YELLOW, COLOUR_SLATE,
						   COLOUR_YELLOW, COLOUR_YELLOW, COLOUR_L_PURPLE}},

	{'y', "Yellow", {11, 1, 11, COLOUR_L_WHITE, COLOUR_L_YELLOW, COLOUR_L_WHITE,
					 COLOUR_WHITE, COLOUR_WHITE, COLOUR_YELLOW}},

	{'R', "Light Red", {12, 1, 12, COLOUR_L_WHITE, COLOUR_YELLOW, COLOUR_RED,
						COLOUR_YELLOW, COLOUR_YELLOW, COLOUR_L_RED}},

	{'G', "Light Green", {13, 1, 13, COLOUR_L_WHITE, COLOUR_YELLOW, COLOUR_GREEN,
						  COLOUR_YELLOW, COLOUR_YELLOW, COLOUR_L_GREEN}},

	{'B', "Light Blue", {14, 1, 14, COLOUR_L_WHITE, COLOUR_YELLOW, COLOUR_BLUE,
						 COLOUR_YELLOW, COLOUR_YELLOW, COLOUR_L_BLUE}},

	{'U', "Light Umber", {15, 1, 15, COLOUR_L_WHITE, COLOUR_YELLOW, COLOUR_UMBER,
						  COLOUR_YELLOW, COLOUR_YELLOW, COLOUR_L_UMBER}},

	/* "new" colors */
	{'p', "Purple", {16, 1, 10,COLOUR_SLATE, COLOUR_L_PURPLE, COLOUR_SLATE,
					 COLOUR_L_PURPLE, COLOUR_L_PURPLE, COLOUR_L_PURPLE}},

	{'v', "Violet", {17, 1, 10,COLOUR_SLATE, COLOUR_L_PURPLE, COLOUR_SLATE,
					 COLOUR_L_PURPLE, COLOUR_L_PURPLE, COLOUR_L_PURPLE}},

	{'t', "Teal", {18, 1, 6, COLOUR_SLATE, COLOUR_L_TEAL, COLOUR_SLATE,
				   COLOUR_L_TEAL, COLOUR_L_TEAL, COLOUR_L_BLUE}},

	{'m', "Mud", {19, 1, 5, COLOUR_SLATE, COLOUR_MUSTARD, COLOUR_SLATE,
				  COLOUR_MUSTARD, COLOUR_MUSTARD, COLOUR_UMBER}},

	{'Y', "Light Yellow", {20, 1, 11, COLOUR_WHITE, COLOUR_WHITE, COLOUR_YELLOW,
						   COLOUR_WHITE, COLOUR_WHITE, COLOUR_L_YELLOW}},

	{'i', "Magenta-Pink", {21, 1, 12, COLOUR_SLATE, COLOUR_L_PINK, COLOUR_RED,
						   COLOUR_L_PINK, COLOUR_L_PINK, COLOUR_L_PURPLE}},

	{'T', "Light Teal", {22, 1, 14, COLOUR_L_WHITE, COLOUR_YELLOW, COLOUR_TEAL,
						 COLOUR_YELLOW, COLOUR_YELLOW, COLOUR_L_BLUE}},

	{'V', "Light Violet", {23, 1, 10, COLOUR_L_WHITE, COLOUR_YELLOW, COLOUR_VIOLET,
						   COLOUR_YELLOW, COLOUR_YELLOW, COLOUR_L_PURPLE}},

	{'I', "Light Pink", {24, 1, 12, COLOUR_L_WHITE, COLOUR_YELLOW, COLOUR_MAGENTA,
						 COLOUR_YELLOW, COLOUR_YELLOW, COLOUR_L_PURPLE}},

	{'M', "Mustard", {25, 1, 11, COLOUR_SLATE, COLOUR_YELLOW, COLOUR_SLATE,
					  COLOUR_YELLOW, COLOUR_YELLOW, COLOUR_YELLOW}},

	{'z', "Blue Slate",  {26, 1, 9, COLOUR_SLATE, COLOUR_DEEP_L_BLUE, COLOUR_SLATE,
						  COLOUR_DEEP_L_BLUE, COLOUR_DEEP_L_BLUE, COLOUR_L_WHITE}},

	{'Z', "Deep Light Blue", {27, 1, 14, COLOUR_L_WHITE, COLOUR_L_BLUE, COLOUR_BLUE_SLATE,
							  COLOUR_L_BLUE, COLOUR_L_BLUE, COLOUR_L_BLUE}},

	/* Rest to be filled in when the game loads */
};



/**
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
	if (c == '\0' || c == ' ') return (COLOUR_DARK);

	/* Search the color table */
	for (a = 0; a < BASIC_COLORS; a++)
	{
		/* Look for the index */
		if (color_table[a].index_char == c) break;
	}

	/* If we don't find the color, we assume white */
	if (a == BASIC_COLORS) return (COLOUR_WHITE);

	/* Return the color */
	return (a);
}


/**
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
	return (COLOUR_WHITE);
}


/**
 * Extract a textual representation of an attribute
 */
const char *attr_to_text(uint8_t a)
{
	if (a < BASIC_COLORS)
		return (color_table[a].name);
	else
		return ("Icky");
}

/**
 * Translate text colours.
 *
 * This translates a color based on the attribute. We use this to set terrain to
 * be lighter or darker, make metallic monsters shimmer, highlight text under
 * the mouse, and reduce the colours on mono colour or 16 colour terms to the
 * correct colour space.
 *
 * TODO: Honour the attribute for the term (full color, mono, 16 color) but
 * ensure that e.g. the lighter version of yellow becomes white in a 16 color
 * term, but light yellow in a full colour term.
 */
uint8_t get_color(uint8_t a, int attr, int n)
{
	/* Accept any graphical attr (high bit set) */
	if (a & (0x80))
		return (a);

	/* TODO: Honour the attribute for the term (full color, mono, 16 color) */
	if (!attr)
		return (a);

	/* Translate the color N times */
	while (n > 0) {
		a = color_table[a].color_translate[attr];
		n--;
	}

	/* Return the modified color */
	return (a);
}


/**
 * XXX XXX XXX Important note about "colors" XXX XXX XXX
 *
 * The "COLOUR_*" color definitions list the "composition" of each
 * "Angband color" in terms of "quarters" of each of the three color
 * components (Red, Green, Blue), for example, COLOUR_UMBER is defined
 * as 2/4 Red, 1/4 Green, 0/4 Blue.
 *
 * These values are NOT gamma-corrected.  On most machines (with the
 * Macintosh being an important exception), you must "gamma-correct"
 * the given values, that is, "correct for the intrinsic non-linearity
 * of the phosphor", by converting the given intensity levels based
 * on the "gamma" of the target screen, which is usually 1.7 (or 1.5).
 *
 * The actual formula for conversion is unknown to me at this time,
 * but you can use the table below for the most common gamma values.
 *
 * So, on most machines, simply convert the values based on the "gamma"
 * of the target screen, which is usually in the range 1.5 to 1.7, and
 * usually is closest to 1.7.  The converted value for each of the five
 * different "quarter" values is given below:
 *
 *   Given  | Gamma 1.0 | Gamma 1.5 | Gamma 1.7 | Hex 1.7
 *  :-----: | :-------: | :-------: | :-------: | :-----:
 *    0/4   |    0.00   |    0.00   |    0.00   |   0x00
 *    1/4   |    0.25   |    0.27   |    0.28   |   0x47
 *    2/4   |    0.50   |    0.55   |    0.56   |   0x8f
 *    3/4   |    0.75   |    0.82   |    0.84   |   0xd7
 *    4/4   |    1.00   |    1.00   |    1.00   |   0xff
 */

/**
 * Table of gamma values
 */
uint8_t gamma_table[256];

/**
 * Table of ln(x / 256) * 256 for x going from 0 -> 255
 */
static const int16_t gamma_helper[256] =
{
	0, -1420, -1242, -1138, -1065, -1007, -961, -921, -887, -857, -830,
	-806, -783, -762, -744, -726, -710, -694, -679, -666, -652, -640,
	-628, -617, -606, -596, -586, -576, -567, -577, -549, -541, -532,
	-525, -517, -509, -502, -495, -488, -482, -475, -469, -463, -457,
	-451, -455, -439, -434, -429, -423, -418, -413, -408, -403, -398,
	-394, -389, -385, -380, -376, -371, -367, -363, -359, -355, -351,
	-347, -343, -339, -336, -332, -328, -325, -321, -318, -314, -311,
	-308, -304, -301, -298, -295, -291, -288, -285, -282, -279, -276,
	-273, -271, -268, -265, -262, -259, -257, -254, -251, -248, -246,
	-243, -241, -238, -236, -233, -231, -228, -226, -223, -221, -219,
	-216, -214, -212, -209, -207, -205, -203, -200, -198, -196, -194,
	-192, -190, -188, -186, -184, -182, -180, -178, -176, -174, -172,
	-170, -168, -166, -164, -162, -160, -158, -156, -155, -153, -151,
	-149, -147, -146, -144, -142, -140, -139, -137, -135, -134, -132,
	-130, -128, -127, -125, -124, -122, -120, -119, -117, -116, -114,
	-112, -111, -109, -108, -106, -105, -103, -102, -100, -99, -97, -96,
	-95, -93, -92, -90, -89, -87, -86, -85, -83, -82, -80, -79, -78,
	-76, -75, -74, -72, -71, -70, -68, -67, -66, -65, -63, -62, -61,
	-59, -58, -57, -56, -54, -53, -52, -51, -50, -48, -47, -46, -45,
	-44, -42, -41, -40, -39, -38, -37, -35, -34, -33, -32, -31, -30,
	-29, -27, -26, -25, -24, -23, -22, -21, -20, -19, -18, -17, -16,
	-14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1
};


/**
 * Build the gamma table so that floating point isn't needed.
 *
 * Note gamma goes from 0->256.  The old value of 100 is now 128.
 */
void build_gamma_table(int gamma)
{
	int i, n;

	/*
	 * value is the current sum.
	 * diff is the new term to add to the series.
	 */
	long value, diff;

	/* Hack - convergence is bad in these cases. */
	gamma_table[0] = 0;
	gamma_table[255] = 255;

	for (i = 1; i < 255; i++) {
		/*
		 * Initialise the Taylor series
		 *
		 * value and diff have been scaled by 256
		 */
		n = 1;
		value = 256L * 256L;
		diff = ((long)gamma_helper[i]) * (gamma - 256);

		while (diff) {
			value += diff;
			n++;

			/*
			 * Use the following identiy to calculate the gamma table.
			 * exp(x) = 1 + x + x^2/2 + x^3/(2*3) + x^4/(2*3*4) +...
			 *
			 * n is the current term number.
			 *
			 * The gamma_helper array contains a table of
			 * ln(x/256) * 256
			 * This is used because a^b = exp(b*ln(a))
			 *
			 * In this case:
			 * a is i / 256
			 * b is gamma.
			 *
			 * Note that everything is scaled by 256 for accuracy,
			 * plus another factor of 256 for the final result to
			 * be from 0-255.  Thus gamma_helper[] * gamma must be
			 * divided by 256*256 each itteration, to get back to
			 * the original power series.
			 */
			diff = (((diff / 256) * gamma_helper[i]) *
					(gamma - 256)) / (256 * n);
		}

		/*
		 * Store the value in the table so that the
		 * floating point pow function isn't needed.
		 */
		gamma_table[i] = (uint8_t)(((long)(value / 256) * i) / 256);
	}
}

