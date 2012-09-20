/* File: main-dos.c */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */


/*
 * This file helps Angband work with DOS computers.
 *
 * This file requires the (free) DJGPP compiler (based on "gcc").
 * See "http://www.delorie.com/djgpp/".
 *
 * This file uses the (free) "Allegro" library (SVGA graphics library).
 * See "http://www.talula.demon.co.uk/allegro/".
 *
 * To compile this file, use "Makefile.dos", which defines "USE_DOS".
 *
 * See also "main-ibm.c" and "main-win.c".
 *
 *
 * The "lib/user/pref-win.prf" file contains macro definitions and possible
 * alternative color set definitions.
 *
 * The "lib/user/font-dos.prf" contains attr/char mappings for use with the
 * special fonts in the "lib/xtra/font/" directory.
 *
 * The "lib/user/graf-win.prf" contains attr/char mappings for use with the
 * special bitmaps in the "lib/xtra/graf/" directory.
 *
 *
 * Both "shift" keys are treated as "identical", and all the modifier keys
 * (control, shift, alt) are ignored when used with "normal" keys, unless
 * they modify the underlying "ascii" value of the key.  You must use the
 * new "user pref files" to be able to interact with the keypad and such.
 *
 * Note the "Term_user_dos()" function hook, which could allow the user
 * to interact with the "main-dos.c" visual system.  Currently this hook
 * is unused, but, for example, it could allow the user to toggle "sound"
 * or "graphics" modes, or to select the number of screen rows, with the
 * extra screen rows being used for the mirror window.
 *
 * Note that "Term_xtra_dos_react()" allows runtime color modification.
 *
 * On my Windows NT box, modes "VESA1" and "VESA2B" seem to work, but
 * result in the game running with no visible output.  Mode "VESA2L"
 * clears the screen and then fails silently.  All other modes fail
 * instantly.  To recover from such "invisible" modes, you can try
 * typing escape, plus control-x, plus escape.  XXX XXX XXX
 *
 *
 * Adapted from "main-ibm.c".
 *
 * Initial framework (and some code) by Ben Harrison (benh@phial.com).
 *
 * Allegro support by Robert Ruehlmann (rr9@inf.tu-dresden.de).
 * See "http://www.inf.tu-dresden.de/~rr9/angband.html".
 */



#include "angband.h"


#ifdef USE_DOS


#include <allegro.h>

#include <bios.h>
#include <dos.h>


/*
 * Index of the first standard Angband color.
 *
 * All colors below this index are defined by
 * the palette of the tiles-bitmap.
 */
#define COLOR_OFFSET 240


/*
 * Maximum number of terminals
 */
#define MAX_TERM_DATA 8


/*
 * Forward declare
 */
typedef struct term_data term_data;

/*
 * Extra "term" data
 */
struct term_data
{
	term t;

	int x;
	int y;

	int cols;
	int rows;

	int tile_wid;
	int tile_hgt;

	int font_wid;
	int font_hgt;

	FONT *font;

#ifdef USE_GRAPHICS

	BITMAP *tiles;

#endif /* USE_GRAPHICS */

};


/*
 * An array of term_data's
 */
static term_data data[MAX_TERM_DATA];


/*
 * Small bitmap for the cursor
 */
static BITMAP *cursor;


/*
 * List of the available videomodes to reduce executable size
 */
DECLARE_GFX_DRIVER_LIST(
	GFX_DRIVER_VBEAF
	GFX_DRIVER_VESA2L
	GFX_DRIVER_VESA2B
	GFX_DRIVER_ATI
	GFX_DRIVER_MACH64
	GFX_DRIVER_CIRRUS64
	GFX_DRIVER_CIRRUS54
	GFX_DRIVER_PARADISE
	GFX_DRIVER_S3
	GFX_DRIVER_TRIDENT
	GFX_DRIVER_ET3000
	GFX_DRIVER_ET4000
	GFX_DRIVER_VIDEO7
	GFX_DRIVER_VESA1
)


/*
 * Declare the videomode list
 */
DECLARE_COLOR_DEPTH_LIST(COLOR_DEPTH_8)



/*
 * Keypress input modifier flags (hard-coded by DOS)
 */
#define K_RSHIFT	0	/* Right shift key down */
#define K_LSHIFT	1	/* Left shift key down */
#define K_CTRL		2	/* Ctrl key down */
#define K_ALT		3	/* Alt key down */
#define K_SCROLL	4	/* Scroll lock on */
#define K_NUM		5	/* Num lock on */
#define K_CAPS		6	/* Caps lock on */
#define K_INSERT	7	/* Insert on */



/*
 * Process an event (check for a keypress)
 *
 * The keypress processing code is often the most system dependant part
 * of Angband, since sometimes even the choice of compiler is important.
 *
 * For this file, we divide all keypresses into two catagories, first, the
 * "normal" keys, including all keys required to play Angband, and second,
 * the "special" keys, such as keypad keys, function keys, and various keys
 * used in combination with various modifier keys.
 *
 * To simplify this file, we use Angband's "macro processing" ability, in
 * combination with the "lib/user/pref-win.prf" file, to handle most of the
 * "special" keys, instead of attempting to fully analyze them here.  This
 * file only has to determine when a "special" key has been pressed, and
 * translate it into a simple string which signals the use of a "special"
 * key, the set of modifiers used, if any, and the hardware scan code of
 * the actual key which was pressed.  To simplify life for the user, we
 * treat both "shift" keys as identical modifiers.
 *
 * The final encoding is "^_MMMxSS\r", where "MMM" encodes the modifiers
 * ("C" for control, "S" for shift, "A" for alt, or any ordered combination),
 * and "SS" encodes the keypress (as the two "digit" hexidecimal encoding of
 * the scan code of the key that was pressed), and the "^_" and "x" and "\r"
 * delimit the encoding for recognition by the macro processing code.
 *
 * Some important facts about scan codes follow.  All "normal" keys use
 * scan codes from 1-58.  The "function" keys use 59-68 (and 133-134).
 * The "keypad" keys use 69-83.  Escape uses 1.  Enter uses 28.  Control
 * uses 29.  Left Shift uses 42.  Right Shift uses 54.  PrtScrn uses 55.
 * Alt uses 56.  Space uses 57.  CapsLock uses 58.  NumLock uses 69.
 * ScrollLock uses 70.  The "keypad" keys which use scan codes 71-83
 * are ordered KP7,KP8,KP9,KP-,KP4,KP5,KP6,KP+,KP1,KP2,KP3,INS,DEL.
 *
 * Using "bioskey(0x10)" instead of "bioskey(0)" apparently provides more
 * information, including better access to the keypad keys in combination
 * with various modifiers, but only works on "PC's after 6/1/86", and there
 * is no way to determine if the function is provided on a machine.  I have
 * been told that without it you cannot detect, for example, control-left.
 * The basic scan code + ascii value pairs returned by the keypad follow,
 * with values in parentheses only available to "bioskey(0x10)".
 *
 *         /      *      -      +      1      2      3      4
 * Norm:  352f   372a   4a2d   4e2b   4f00   5000   5100   4b00
 * Shft:  352f   372a   4a2d   4e2b   4f31   5032   5133   4b34
 * Ctrl: (9500) (9600) (8e00) (9000)  7500  (9100)  7600   7300
 *
 *         5      6      7      8      9      0      .     Enter
 * Norm: (4c00)  4d00   4700   4800   4900   5200   5300  (e00d)
 * Shft:  4c35   4d36   4737   4838   4939   5230   532e  (e00d)
 * Ctrl: (8f00)  7400   7700  (8d00)  8400  (9200) (9300) (e00a)
 *
 * See "lib/user/pref-win.prf" for the "standard" macros for various keys.
 *
 * Certain "bizarre" keypad keys (such as "enter") return a "scan code"
 * of "0xE0", and a "usable" ascii value.  These keys should be treated
 * like the normal keys, see below.  XXX XXX XXX Note that these "special"
 * keys could be prefixed with an optional "ctrl-^" which would allow them
 * to be used in macros without hurting their use in normal situations.
 *
 * This function also appears in "main-ibm.c".  XXX XXX XXX
 */
static errr Term_xtra_dos_event(int v)
{
	int i, k, s;

	bool mc = FALSE;
	bool ms = FALSE;
	bool ma = FALSE;

	/* Hack -- Check for a keypress */
	if (!v && !bioskey(1)) return (1);

	/* Wait for a keypress */
	k = bioskey(0x10);

	/* Access the "modifiers" */
	i = bioskey(2);

	/* Extract the "scan code" */
	s = ((k >> 8) & 0xFF);

	/* Extract the "ascii value" */
	k = (k & 0xFF);

	/* Process "normal" keys */
	if ((s <= 58) || (s == 0xE0))
	{
		/* Enqueue it */
		if (k) Term_keypress(k);

		/* Success */
		return (0);
	}

	/* Extract the modifier flags */
	if (i & (1 << K_CTRL)) mc = TRUE;
	if (i & (1 << K_LSHIFT)) ms = TRUE;
	if (i & (1 << K_RSHIFT)) ms = TRUE;
	if (i & (1 << K_ALT)) ma = TRUE;

	/* Begin a "macro trigger" */
	Term_keypress(31);

	/* Hack -- Send the modifiers */
	if (mc) Term_keypress('C');
	if (ms) Term_keypress('S');
	if (ma) Term_keypress('A');

	/* Introduce the hexidecimal scan code */
	Term_keypress('x');

	/* Encode the hexidecimal scan code */
	Term_keypress(hexsym[s/16]);
	Term_keypress(hexsym[s%16]);

	/* End the "macro trigger" */
	Term_keypress(13);

	/* Success */
	return (0);
}


/*
 * Set the Angband pallete
 */
void Term_xtra_dos_react(void)
{
	int i;

	/* Set the Angband colors */
	for (i = 0; i < 16; i++)
	{
		/* Extract desired values */
		char rv = angband_color_table[i][1] >> 2;
		char gv = angband_color_table[i][2] >> 2;
		char bv = angband_color_table[i][3] >> 2;

		RGB color = { rv,  gv,  bv  };

		set_color(COLOR_OFFSET + i, &color);
	}
}


/*
 * Clear a terminal
 */
void Term_xtra_dos_clear(void)
{
	term_data *td = (term_data*)(Term->data);

	int x1, y1;
	int w1, h1;

	/* Location */
	x1 = td->x;
	y1 = td->y;

	/* Size */
	w1 = td->tile_wid * td->cols;
	h1 = td->tile_hgt * td->rows;

	/* Draw the Term black */
	rectfill(screen,
	         x1, y1, x1 + w1 - 1, y1 + h1 - 1,
	         COLOR_OFFSET + TERM_DARK);
}


/*
 * Handle a "special request"
 *
 * The given parameters are "valid".
 */
static errr Term_xtra_dos(int n, int v)
{
	/* Analyze the request */
	switch (n)
	{
		/* Make a "bell" noise */
		case TERM_XTRA_NOISE:
		{
			/* Make a bell noise */
			(void)write(1, "\007", 1);

			/* Success */
			return (0);
		}

		/* Clear the screen */
		case TERM_XTRA_CLEAR:
		{
			Term_xtra_dos_clear();

			/* Success */
			return (0);
		}

		/* Process events */
		case TERM_XTRA_EVENT:
		{
			/* Process one event */
			return (Term_xtra_dos_event(v));
		}

		/* Flush events */
		case TERM_XTRA_FLUSH:
		{
			/* Strip events */
			while (!Term_xtra_dos_event(FALSE));

			/* Success */
			return (0);
		}

		/* React to global changes */
		case TERM_XTRA_REACT:
		{
			/* Change the colors */
			Term_xtra_dos_react();

			/* Success */
			return (0);
		}

		/* Delay for some milliseconds */
		case TERM_XTRA_DELAY:
		{
			/* Delay if needed */
			if (v > 0) delay(v);

			/* Success */
			return (0);
		}
	}

	/* Unknown request */
	return (1);
}



/*
 * Move the cursor
 *
 * The given parameters are "valid".
 */
static errr Term_curs_dos(int x, int y)
{
	term_data *td = (term_data*)(Term->data);

	int x1, y1;

	/* Location */
	x1 = x * td->tile_wid + td->x;
	y1 = y * td->tile_hgt + td->y;

	/* Draw the cursor */
	draw_sprite(screen, cursor, x1, y1);

	/* Success */
	return (0);
}


/*
 * Erase a block of the screen
 *
 * The given parameters are "valid".
 */
static errr Term_wipe_dos(int x, int y, int n)
{
	term_data *td = (term_data*)(Term->data);

	int x1, y1;
	int w1, h1;

	/* Location */
	x1 = x * td->tile_wid + td->x;
	y1 = y * td->tile_hgt + td->y;

	/* Size */
	w1 = n * td->tile_wid;
	h1 = td->tile_hgt;

	/* Draw a black block */
	rectfill(screen, x1, y1, x1 + w1 - 1, y1 + h1 - 1,
	         COLOR_OFFSET + TERM_DARK);

	/* Success */
	return (0);
}


/*
 * Place some text on the screen using an attribute
 *
 * The given parameters are "valid".  Be careful with "a".
 *
 * The string "cp" has length "n" and is NOT null-terminated.
 */
static errr Term_text_dos(int x, int y, int n, byte a, const char *cp)
{
	term_data *td = (term_data*)(Term->data);

	int i;

	int x1, y1;

	char text[257];

	/* Location */
	x1 = x * td->tile_wid + td->x;
	y1 = y * td->tile_hgt + td->y;

	/* Erase old contents */
	Term_wipe_dos(x, y, n);

	/* No stretch needed */
	if (td->font_wid == td->tile_wid)
	{
		/* Copy the string */
		for (i = 0; i < n; ++i) text[i] = cp[i];

		/* Terminate */
		text[i] = '\0';

		/* Dump the text */
		textout(screen, td->font, text, x1, y1,
		       	COLOR_OFFSET + (a & 0x0F));
	}

	/* Stretch needed */
	else
	{
		/* Pre-Terminate */
		text[1] = '\0';

		/* Write the chars to the screen */
		for (i = 0; i < n; ++i)
		{
			/* Build a one character string */
			text[0] = cp[i];

			/* Dump some text */
			textout(screen, td->font, text, x1, y1,
		        	COLOR_OFFSET + (a & 0x0F));

			/* Advance */
			x1 += td->tile_wid;
		}
	}

	/* Success */
	return (0);
}


#ifdef USE_GRAPHICS

/*
 * Place some attr/char pairs on the screen
 *
 * The given parameters are "valid".
 *
 * To prevent crashes, we must not only remove the high bits of the
 * "ap[i]" and "cp[i]" values, but we must map the resulting value
 * onto the legal bitmap size, which is normally 32x32.  XXX XXX XXX
 */
static errr Term_pict_dos(int x, int y, int n, const byte *ap, const char *cp)
{
	term_data *td = (term_data*)(Term->data);

	int i;

	int w, h;

	int x1, y1;
	int x2, y2;

	/* Size */
	w = td->tile_wid;
	h = td->tile_hgt;

	/* Location (window) */
	x1 = x * w + td->x;
	y1 = y * h + td->y;

	/* Dump the tiles */
	for (i = 0; i < n; i++)
	{
		/* Location (bitmap) */
		x2 = (cp[i] & 0x1F) * w;
		y2 = (ap[i] & 0x1F) * h;

		/* Blit the tile to the screen */
		blit(td->tiles, screen, x2, y2, x1, y1, w, h);

		/* Advance (window) */
		x1 += w;
	}

	/* Success */
	return (0);
}

#endif /* USE_GRAPHICS */


/*
 * Init a Term
 */
static void Term_init_dos(term *t)
{
	/* XXX Nothing */
}


/*
 * Nuke a Term
 */
static void Term_nuke_dos(term *t)
{
	term_data *td = (term_data*)(t->data);

	/* Free the terminal font */
	if (td->font) destroy_font(td->font);

#ifdef USE_GRAPHICS

	/* Free the terminal bitmap */
	if (td->tiles) destroy_bitmap(td->tiles);

#endif /* USE_GRAPHICS */

}



/*
 * Instantiate a "term_data" structure
 */
static void term_data_link(term_data *td)
{
	term *t = &td->t;

	/* Initialize the term */
	term_init(t, td->cols, td->rows, 255);

	/* Use a "software" cursor */
	t->soft_cursor = TRUE;

	/* Ignore the "TERM_XTRA_BORED" action */
	t->never_bored = TRUE;

	/* Ignore the "TERM_XTRA_FROSH" action */
	t->never_frosh = TRUE;

	/* Erase with "white space" */
	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';

	/* Prepare the init/nuke hooks */
	t->init_hook = Term_init_dos;
	t->nuke_hook = Term_nuke_dos;

	/* Prepare the template hooks */
	t->xtra_hook = Term_xtra_dos;
	t->curs_hook = Term_curs_dos;
	t->wipe_hook = Term_wipe_dos;
	t->text_hook = Term_text_dos;

#ifdef USE_GRAPHICS

	/* Graphics */
	if (use_graphics)
	{
		/* Prepare the graphics hook */
		t->pict_hook = Term_pict_dos;

		/* Use "Term_pict" for "graphic" data */
		t->higher_pict = TRUE;
	}

#endif /* USE_GRAPHICS */

	/* Remember where we came from */
	t->data = (vptr)(td);
}


/*
 * Shut down visual system, then fall back into standard "quit()"
 */
static void quit_hook(cptr str)
{
	int i;


	/* Destroy sub-windows */
	for (i = MAX_TERM_DATA - 1; i >= 1; i--)
	{
		/* Unused */
		if (!angband_term[i]) continue;

		/* Nuke it */
		term_nuke(angband_term[i]);
	}


	/* Free all resources */
	if (cursor) destroy_bitmap(cursor);

	/* Shut down Allegro */
	allegro_exit();
}



/*
 * GRX font file reader by Mark Wodrich.
 *
 * GRX FNT files consist of the header data (see struct below). If the font
 * is proportional, followed by a table of widths per character (unsigned
 * shorts). Then, the data for each character follows. 1 bit/pixel is used,
 * with each line of the character stored in contiguous bytes. High bit of
 * first byte is leftmost pixel of line.
 *
 * Note that GRX FNT files can have a variable number of characters, so you
 * must verify that any "necessary" characters exist before using them.
 *
 * The GRX FNT files were developed by ???.
 */


/*
 * Magic value
 */
#define FONTMAGIC	0x19590214L


/*
 * Forward declare
 */
typedef struct FNTfile_header FNTfile_header;


/*
 * .FNT file header
 */
struct FNTfile_header
{
	unsigned long  magic;
	unsigned long  bmpsize;
	unsigned short width;
	unsigned short height;
	unsigned short minchar;
	unsigned short maxchar;
	unsigned short isfixed;
	unsigned short reserved;
	unsigned short baseline;
	unsigned short undwidth;
	char           fname[16];
	char           family[16];
};


/*
 * A "bitmap" is simply an array of bytes
 */
typedef byte *GRX_BITMAP;


/*
 * Temporary space to store font bitmap
 */
#define GRX_TMP_SIZE    4096


/*
 * ???
 */
void convert_grx_bitmap(int width, int height, GRX_BITMAP src, GRX_BITMAP dest)
{
	unsigned short x, y, bytes_per_line;
	unsigned char bitpos, bitset;

	bytes_per_line = (width+7) >> 3;

	for (y=0; y<height; y++)
	{
		for (x=0; x<width; x++)
		{
			bitpos = 7-(x&7);
			bitset = !!(src[(bytes_per_line*y) + (x>>3)] & (1<<bitpos));
			dest[y*width+x] = bitset;
		}
	}
}


/*
 * ???
 */
GRX_BITMAP *load_grx_bmps(PACKFILE *f, FNTfile_header *hdr,
                          int numchar, unsigned short *wtable)
{
	int t, width, bmp_size;
	GRX_BITMAP temp;
	GRX_BITMAP *bmp;

	/* alloc array of bitmap pointers */
	bmp = malloc(sizeof(GRX_BITMAP) * numchar);

	/* assume it's fixed width for now */
	width = hdr->width;

	/* temporary working area to store FNT bitmap */
	temp = malloc(GRX_TMP_SIZE);

	for (t=0; t<numchar; t++)
	{
		/* if prop. get character width */
		if (!hdr->isfixed) width = wtable[t];

		/* work out how many bytes to read */
		bmp_size = ((width+7) >> 3) * hdr->height;

		/* oops, out of space! */
		if (bmp_size > GRX_TMP_SIZE)
		{
	 		free(temp);
	 		for (t--; t>=0; t--) free(bmp[t]);
	 		free(bmp);
	 		return NULL;
		}

		/* alloc space for converted bitmap */
		bmp[t] = malloc(width*hdr->height);

		/* read data */
		pack_fread(temp, bmp_size, f);

		/* convert to 1 byte/pixel */
		convert_grx_bitmap(width, hdr->height, temp, bmp[t]);
	}

	free(temp);
	return bmp;
}


/*
 * ???
 */
FONT *import_grx_font(char *fname)
{
	PACKFILE *f;

	/* GRX font header */
	FNTfile_header hdr;

	/* number of characters in the font */
	int numchar;

	/* table of widths for each character */
	unsigned short *wtable = NULL;

	/* array of font bitmaps */
	GRX_BITMAP *bmp;

	/* the Allegro font */
	FONT *font = NULL;

	FONT_PROP *font_prop;
	int c, c2, start, width;


	f = pack_fopen(fname, F_READ);

	if (!f) return NULL;

	/* read the header structure */
	pack_fread(&hdr, sizeof(hdr), f);

	/* check magic number */
	if (hdr.magic != FONTMAGIC)
	{
		pack_fclose(f);
		return NULL;
	}

	numchar = hdr.maxchar-hdr.minchar+1;

	/* proportional font */
	if (!hdr.isfixed)
	{
		wtable = malloc(sizeof(unsigned short) * numchar);
		pack_fread(wtable, sizeof(unsigned short) * numchar, f);
	}

	bmp = load_grx_bmps(f, &hdr, numchar, wtable);

	if (!bmp) goto get_out;

	if (pack_ferror(f)) goto get_out;

	font = malloc(sizeof(FONT));
	font->height = -1;
	font->dat.dat_prop = font_prop = malloc(sizeof(FONT_PROP));

	start = 32 - hdr.minchar;
	width = hdr.width;

	for (c=0; c<FONT_SIZE; c++)
	{
		c2 = c+start;

		if ((c2 >= 0) && (c2 < numchar))
		{
			if (!hdr.isfixed) width = wtable[c2];

			font_prop->dat[c] = create_bitmap_ex(8, width, hdr.height);
			memcpy(font_prop->dat[c]->dat, bmp[c2], width*hdr.height);
		}
		else
		{
			font_prop->dat[c] = create_bitmap_ex(8, 8, hdr.height);
			clear(font_prop->dat[c]);
		}
	}

get_out:

	pack_fclose(f);

	if (wtable) free(wtable);

	if (bmp)
	{
		for (c=0; c<numchar; c++) free(bmp[c]);

		free(bmp);
	}

	return font;
}


/*
 * Attempt to initialize this file
 *
 * Hack -- we assume that "blank space" should be "white space"
 * (and not "black space" which might make more sense).
 *
 * Note the use of "((x << 2) | (x >> 4))" to "expand" a 6 bit value
 * into an 8 bit value, without losing much precision, by using the 2
 * most significant bits as the least significant bits in the new value.
 *
 * We should attempt to "share" bitmaps (and fonts) between windows
 * with the same "tile" size.  XXX XXX XXX
 */
errr init_dos(void)
{
	int i, num_windows;

	term_data *td;

	char section[80];

	int screen_wid;
	int screen_hgt;

	char name_tiles[128];

	char name_fonts[8][128];

	char filename[1024];

#ifdef USE_GRAPHICS

	/* Large bitmap for the tiles */
	BITMAP *tiles = NULL;
	PALLETE tiles_pallete;

#endif /* USE_GRAPHICS */

	/* Size of each bitmap tile */
	int bitmap_wid;
	int bitmap_hgt;

	/* Extra paths */
	char xtra_font_dir[1024];
	char xtra_graf_dir[1024];


	/* Initialize the Allegro library (never fails) */
	(void)allegro_init();


	/* Read config info from filename */
	override_config_file("angdos.cfg");


	/* Main section */
	strcpy(section, "Angband");

	/* Get screen size */
	screen_wid = get_config_int(section, "screen_wid", 640);
	screen_hgt = get_config_int(section, "screen_hgt", 480);

	/* Get bitmap tile size */
	bitmap_wid = get_config_int(section, "bitmap_wid", 8);
	bitmap_hgt = get_config_int(section, "bitmap_hgt", 8);

	/* Get bitmap filename */
	strcpy(name_tiles, get_config_string(section, "bitmap_file", "8x8.bmp"));


	/* Get number of windows */
	num_windows = get_config_int(section, "num_windows", 1);

	/* Paranoia */
	if (num_windows > 8) num_windows = 8;


	/* Init the terms */
	for (i = 0; i < num_windows; i++)
	{
		td = &data[i];
		WIPE(td, term_data);

		/* Section name */
		sprintf(section, "Term-%d", i);

		/* Coordinates of left top corner */
		td->x = get_config_int(section, "x", 0);
		td->y = get_config_int(section, "y", 0);

		/* Rows and cols of term */
		td->rows = get_config_int(section, "rows", 24);
		td->cols = get_config_int(section, "cols", 80);

		/* Tile size */
		td->tile_wid = get_config_int(section, "tile_wid", 8);
		td->tile_hgt = get_config_int(section, "tile_hgt", 12);

		/* Font size */
		td->font_wid = get_config_int(section, "tile_wid", 8);
		td->font_hgt = get_config_int(section, "tile_hgt", 12);

		/* Get font filename */
		strcpy(name_fonts[i], get_config_string(section, "font_file", "xm8x12.fnt"));
	}


	/* Set the color depth */
	set_color_depth(8);

	/* Auto-detect, and instantiate, the appropriate graphics mode */
	if ((set_gfx_mode(GFX_AUTODETECT, screen_wid, screen_hgt, 0, 0)) < 0)
	{
		char error_text[1024];

		/* Save the Allegro error description */
		strcpy(error_text, allegro_error);

		/* Shut down Allegro */
		allegro_exit();

		/* Print the error description */
		plog_fmt("Error selecting screen mode: %s", error_text);

		/* Failure */
		return (-1);
	}


	/* Hook in "z-util.c" hook */
	quit_aux = quit_hook;


	/* Build the "graf" path */
	path_build(xtra_graf_dir, 1024, ANGBAND_DIR_XTRA, "graf");

	/* Build the "font" path */
	path_build(xtra_font_dir, 1024, ANGBAND_DIR_XTRA, "font");


#ifdef USE_GRAPHICS

	/* Initialize graphics */
	if (arg_graphics)
	{
		/* Build the name of the bitmap file */
		path_build(filename, 1024, xtra_graf_dir, name_tiles);

		/* Open the bitmap file */
		if ((tiles = load_bitmap(filename, tiles_pallete)) != NULL)
		{
			/* Select the bitmap pallete */
			set_pallete(tiles_pallete);

			/* Use graphics */
			use_graphics = TRUE;
		}
	}

#endif /* USE_GRAPHICS */

	/* Set the Angband colors */
	Term_xtra_dos_react();


	/* Prepare the fonts and graphics */
	for (i = 0; i < num_windows; i++)
	{
		td = &data[i];

		/* Build the name of the font file */
		path_build(filename, 1024, xtra_font_dir, name_fonts[i]);

		/* Load a "*.fnt" file */
		if (suffix(filename, ".fnt"))
		{
			/* Load the font file */
			if (!(td->font = import_grx_font(filename)))
			{
				quit_fmt("Error reading font file '%s'", filename);
			}
		}

		/* Load a "*.dat" file */
		else if (suffix(filename, ".dat"))
		{
			DATAFILE *fontdata;

			/* Load the font file */
			if (!(fontdata = load_datafile(filename)))
			{
				quit_fmt("Error reading font file '%s'", filename);
			}

			/* Save the font data */
			td->font = fontdata[1].dat;

			/* Unload the font file */
			unload_datafile_object(fontdata);
		}

		/* Oops */
		else
		{
			quit_fmt("Unknown suffix in font file '%s'", filename);
		}

#ifdef USE_GRAPHICS

		/* Stretch bitmap */
		if (use_graphics)
		{
			int col, row;

			int src_x, src_y;
			int tgt_x, tgt_y;

			int cols = tiles->w / bitmap_wid;
			int rows = tiles->h / bitmap_hgt;

			int width = td->tile_wid * cols;
			int height = td->tile_hgt * rows;

			/* Initialize the tile graphics */
			td->tiles = create_bitmap(width, height);

			for (row = 0; row < rows; ++row)
			{
				src_y = row * bitmap_hgt;
				tgt_y = row * td->tile_hgt;

				for (col = 0; col < cols; ++col)
				{
					src_x = col * bitmap_wid;
					tgt_x = col * td->tile_wid;

					stretch_blit(tiles, td->tiles,
					             src_x, src_y,
					             bitmap_wid, bitmap_hgt,
					             tgt_x, tgt_y,
					             td->tile_wid, td->tile_hgt);
				}
			}
		}

#endif /* USE_GRAPHICS */


		/* Link the term */
		term_data_link(td);
		angband_term[i] = &td->t;
	}

#ifdef USE_GRAPHICS

	/* free the old tiles bitmap */
	if (tiles) destroy_bitmap(tiles);

#endif /* USE_GRAPHICS */


	/* Main screen */
	td = &data[0];

	/* Build a cursor bitmap */
	cursor = create_bitmap(td->tile_wid, td->tile_hgt);

	/* Erase the cursor sprite */
	clear(cursor);

	/* Draw the cursor sprite (yellow rectangle) */
	rect(cursor, 0, 0, td->tile_wid - 1, td->tile_hgt - 1,
	     COLOR_OFFSET + TERM_YELLOW);


	/* Activate the main term */
	Term_activate(angband_term[0]);

	/* Erase the screen */
	Term_xtra_dos_clear();

	/* Place the cursor */
	Term_curs_dos(0, 0);

	/* Success */
	return 0;
}


#endif /* USE_DOS */


