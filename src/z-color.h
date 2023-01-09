/**
 * \file z-color.h
 * \brief Generic color definitions
 *
 * Copyright (c) 1997 Ben Harrison
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#ifndef INCLUDED_Z_COLOR_H
#define INCLUDED_Z_COLOR_H

#include "h-basic.h"

/**
 * Angband "attributes" (with symbols, and base (R,G,B) codes)
 *
 * The "(R,G,B)" codes are given in "fourths" of the "maximal" value,
 * and should "gamma corrected" on most (non-Macintosh) machines.
 */
#define COLOUR_DARK     0  /* d */    /* 0 0 0 */
#define COLOUR_WHITE    1  /* w */    /* 4 4 4 */
#define COLOUR_SLATE    2  /* s */    /* 2 2 2 */
#define COLOUR_ORANGE   3  /* o */    /* 4 2 0 */
#define COLOUR_RED      4  /* r */    /* 3 0 0 */
#define COLOUR_GREEN    5  /* g */    /* 0 2 1 */
#define COLOUR_BLUE     6  /* b */    /* 0 0 4 */
#define COLOUR_UMBER    7  /* u */    /* 2 1 0 */
#define COLOUR_L_DARK   8  /* D */    /* 1 1 1 */
#define COLOUR_L_WHITE  9  /* W */    /* 3 3 3 */
#define COLOUR_L_PURPLE 10 /* P */    /* ? ? ? */
#define COLOUR_YELLOW   11 /* y */    /* 4 4 0 */
#define COLOUR_L_RED    12 /* R */    /* 4 0 0 */
#define COLOUR_L_GREEN  13 /* G */    /* 0 4 0 */
#define COLOUR_L_BLUE   14 /* B */    /* 0 4 4 */
#define COLOUR_L_UMBER  15 /* U */    /* 3 2 1 */

#define COLOUR_PURPLE      16    /* p */
#define COLOUR_VIOLET      17    /* v */
#define COLOUR_TEAL        18    /* t */
#define COLOUR_MUD         19    /* m */
#define COLOUR_L_YELLOW    20    /* Y */
#define COLOUR_MAGENTA     21    /* i */
#define COLOUR_L_TEAL      22    /* T */
#define COLOUR_L_VIOLET    23    /* V */
#define COLOUR_L_PINK      24    /* I */
#define COLOUR_MUSTARD     25    /* M */
#define COLOUR_BLUE_SLATE  26    /* z */
#define COLOUR_DEEP_L_BLUE 27    /* Z */
#define COLOUR_SHADE       28    /* for shaded backgrounds */

/**
 * The following allow color 'translations' to support environments with a
 * limited color depth as well as translate colours to alternates
 * for e.g. menu highlighting.
 */
#define ATTR_FULL        0    /* full color translation */
#define ATTR_MONO        1    /* mono color translation */
#define ATTR_VGA         2    /* 16 color translation */
#define ATTR_BLIND       3    /* "Blind" color translation */
#define ATTR_LIGHT       4    /* "Torchlit" color translation */
#define ATTR_DARK        5    /* "Dark" color translation */
#define ATTR_HIGH        6    /* "Highlight" color translation */
#define ATTR_METAL       7    /* "Metallic" color translation */
#define ATTR_MISC        8    /* "Miscellaneous" - see misc_to_attr */

#define MAX_ATTR        9

/**
 * Maximum number of colours, and number of "basic" Angband colours
 * Limit the maximum to be less than or equal to 128 since the 7th bit (i.e.
 * 128 or 0x80) in color indices is used to trigger tile rendering with the
 * front ends that set higher_pict to true in struct term.
 */ 
#define MAX_COLORS      32
#define BASIC_COLORS    29
/**
 * This is the multiplier for the BG_* constants.  Must be a multiple of
 * MAX_COLORS (so (c + MULT_BG * BG_x) % MAX_COLORS is equal to c for 0 <= c
 * < MAX_COLORS) and (MULT_BG * BG_x) & 0x80 must be zero to avoid triggering
 * tile rendering with the front ends that set higher_pict to true in
 * struct term.
 */
#define MULT_BG 256
#define BG_BLACK 0	/* The set number for the black-background glyphs */
#define BG_SAME  1	/* The set number for the same-background glyphs */
#define BG_DARK  2	/* The set number for the dark-background glyphs */
#define BG_MAX   3	/* The max number of backgrounds */

/**
 * A game color.
 */
typedef struct color_type color_type;
struct color_type
{
	char index_char;            /* Character index:  'r' = red, etc. */
	char name[32];              /* Color name */
	uint8_t color_translate[MAX_ATTR]; /* Index for various in-game translations */
};

extern uint8_t angband_color_table[MAX_COLORS][4];
extern color_type color_table[MAX_COLORS];

extern int color_char_to_attr(char c);
extern int color_text_to_attr(const char *name);
extern const char *attr_to_text(uint8_t a);
extern uint8_t get_color(uint8_t a, int attr, int n);

extern void build_gamma_table(int gamma);
extern uint8_t gamma_table[256];

#endif /* INCLUDED_Z_COLOR_H */

