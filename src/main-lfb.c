/* File: main-lfb.c */

/*
 * Linux framebuffer support for Angband.
 * Copyright (C) 2005 Alexander Ulyanov <posband@earthsea.org>
 * (initial framework by Ben Harrison; BMP image manipulation by
 * Desvignes Sebastien, Denis Eropkin, Ben Harrison; input handling
 * by Alexander Malmberg)
 */

/*
 * Linux framebuffer is a very simple interface for direct access
 * to graphics hardware (available since Linux 2.2).  Basically,
 * you just memory-map the framebuffer device file, get the pointer
 * to the framebuffer and here you go.  Framebuffer is primarily
 * used for a large and neat console, but a number of graphics
 * programs support it as well:
 * 
 *  - X Window (XFree86 and X.Org, to be precise. See fbdev(4).)
 *  - GTK+ software (in theory, at least :).)
 *  - SDL software
 *  - Links (a web browser)
 *  - And of course this shiny new Angband front-end :).
 * 
 * Setting up framebuffer is simple enough.  Actually, chances are it's
 * already set up during Linux installation.  If it isn't, you will
 * have to specify the right boot options for kernel, and probably
 * recompile the kernel.
 *
 * MAKE SURE THAT YOU HAVE THE READ/WRITE ACCESS TO /dev/fb0 DEVICE!!!
 * This is the main source of the problems.  If you are on a local
 * machine, 666 permission for the device shouldn't cause any problems.
 *
 * Some notes on this front-end:
 * 
 *  - It is originally written for PosBand, but it should be very
 *    easy to port to other *bands.  Just replace "posband.h" with
 *    approprite header (usually "angband.h"), and define USE_LFB
 *    for the compilation.
 *  - The front-end, of course, is Linux specific.  It doesn't use any
 *    additional libraries.
 *  - The front-end doesn't try to support 4-bit VGA mode.  It's rather
 *    braindead, and I don't see any reason to use VGA framebuffer
 *    anyway.
 *  - The front-end doesn't try to switch video mode, and uses whatever
 *    is available.  Mode switching is the hardest part of fbdev
 *    programming, and vesafb driver doesn't support it anyway.
 *  - Tiles _are_ supported.
 *  - Fonts are loaded from *.bmp files in lib/xtra/font.  They are
 *    supposed to be in a monochrome BMP, containing 128 characters
 *    in a row.  Kind of tiles :).
 *
 * This code can be distributed and modified freely under the terms
 * of GNU General Public License.  There is NO WARRANTY WHATSOEVER,
 * use at your own risk.
 *
 * --A.U.
 */



#include "angband.h"

#ifdef USE_LFB

/* Front-end version */
#define LFB_VERSION			"20050220"

/*
 * XXX Basic sanity check
 */
#ifndef linux
# error Linux framebuffer is supported only on Linux.
#endif

#include "main.h"

#include <asm/page.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>

#define DEF_FB_DEV	"/dev/fb0"	/* Default device */
#define DEF_FB_FONT	"10x20"		/* Default font */

static const char *fb_name = DEF_FB_DEV;	/* Device name */
static const char *fb_font = DEF_FB_FONT;	/* Font name */
static const char *fb_tile = NULL;	/* Tile file name */
static int fb_fd = -1;			/* File descriptor */
static byte *fb;			/* Mmapped framebuffer */
static size_t fb_len;			/* Length of mmap */
static struct fb_fix_screeninfo fb_finfo;	/* Fixed info */
static struct fb_var_screeninfo fb_vinfo;	/* Variable info */
static s16b s_red[256], s_green[256], s_blue[256];	/* Saved palette */
static byte *font = NULL;		/* Font data */
static size_t fontpitch;		/* Font line length in bytes */
static byte fontw, fonth;		/* Character size */
static byte *tile = NULL;		/* Tile data */
static size_t tilepitch;		/* Tile line length in bytes */
static byte tilew, tileh;		/* Tile size */
static byte pict_wid, pict_hgt;		/* Tile size before rescaling */
static byte cellw, cellh;		/* "Cell" size */

static cptr ANGBAND_DIR_XTRA_FONT;
static cptr ANGBAND_DIR_XTRA_GRAF;

static struct termios  norm_termios;
static struct termios  game_termios;


/*
 * Local palette representation.
 *
 * The "Term_xtra(TERM_XTRA_REACT, 0)" hook is used to initialize
 * "color_data_[rgb]" from "angband_color_table".
 */
static u16b color_data_r[256], color_data_g[256], color_data_b[256];

/*
 * A macro to put the pixel
 */
#define putpixel(x, y, c) do { \
        u32b pixel_value; \
	if (fb_finfo.visual == FB_VISUAL_PSEUDOCOLOR) \
	{ \
	    	pixel_value = c;   \
	} \
	else /* DirectColor/TrueColor */ \
	{ \
		pixel_value = (((u32b)((color_data_r[c] >> 8) >> (8 - fb_vinfo.red.length)) << fb_vinfo.red.offset) | \
			       ((u32b)((color_data_g[c] >> 8) >> (8 - fb_vinfo.green.length)) << fb_vinfo.green.offset) | \
			       ((u32b)((color_data_b[c] >> 8) >> (8 - fb_vinfo.blue.length)) << fb_vinfo.blue.offset)); \
	} \
    	switch (fb_vinfo.bits_per_pixel) \
	{ \
		case 8: \
			*(fb + fb_finfo.line_length * (y) + (x)) = pixel_value; \
			break; \
		case 16: \
			*((u16b *)fb + fb_finfo.line_length / 2 * (y) + (x)) = pixel_value; \
			break; \
		case 24: \
			*(fb + fb_finfo.line_length * (y) + 3 * (x))     = color_data_b[c] >> 8; \
			*(fb + fb_finfo.line_length * (y) + 3 * (x) + 1) = color_data_g[c] >> 8; \
			*(fb + fb_finfo.line_length * (y) + 3 * (x) + 2) = color_data_r[c] >> 8; \
			break; \
		case 32: \
			*((u32b *)fb + fb_finfo.line_length / 4 * (y) + (x)) = pixel_value; \
			break; \
	} \
} while(0)


/*** BMP-related data types ***/

/*
 * The Win32 "BITMAPFILEHEADER" type.
 */
typedef struct BITMAPFILEHEADER
{
	u16b bfType;
	u32b bfSize;
	u16b bfReserved1;
	u16b bfReserved2;
	u32b bfOffBits;
} BITMAPFILEHEADER;

/*
 * The Win32 "BITMAPINFOHEADER" type.
 */
typedef struct BITMAPINFOHEADER
{
	u32b biSize;
	u32b biWidth;
	u32b biHeight;
	u16b biPlanes;
	u16b biBitCount;
	u32b biCompresion;
	u32b biSizeImage;
	u32b biXPelsPerMeter;
	u32b biYPelsPerMeter;
	u32b biClrUsed;
	u32b biClrImportand;
} BITMAPINFOHEADER;

/*
 * The Win32 "RGBQUAD" type.
 */
typedef struct RGBQUAD
{
	unsigned char b, g, r;
	unsigned char filler;
} RGBQUAD;


static errr Term_text_lfb(int x, int y, int n, byte a, const char *cp);


/*** Helper functions for system independent file loading. ***/

static byte get_byte(FILE *fff)
{
	/* Get a character, and return it */
	return (getc(fff) & 0xFF);
}

static void rd_byte(FILE *fff, byte *ip)
{
	*ip = get_byte(fff);
}

static void rd_u16b(FILE *fff, u16b *ip)
{
	(*ip) = get_byte(fff);
	(*ip) |= ((u16b)(get_byte(fff)) << 8);
}

static void rd_u32b(FILE *fff, u32b *ip)
{
	(*ip) = get_byte(fff);
	(*ip) |= ((u32b)(get_byte(fff)) << 8);
	(*ip) |= ((u32b)(get_byte(fff)) << 16);
	(*ip) |= ((u32b)(get_byte(fff)) << 24);
}


/*** Misc. internal functions ***/

void Term_xtra_lfb_react()
{
	struct fb_cmap cmap;
	int i;

	/*
	 * "React" to changes.  This means setting the palette.
	 * In true-color mode we still need the palette to
	 * convert color index to the actual color ourselves.
	 *
	 * A necessary hack -- fbdev's palette is 48-bit
	 * (16 bit for red, green and blue). 
	 */
	for (i = 0; i < 256; i++)
	{
		color_data_r[i] = angband_color_table[i][1] << 8;
		color_data_g[i] = angband_color_table[i][2] << 8;
		color_data_b[i] = angband_color_table[i][3] << 8;

		if (color_data_r[i] == 0xf000) color_data_r[i] = 0xffff;
		if (color_data_g[i] == 0xf000) color_data_g[i] = 0xffff;
		if (color_data_b[i] == 0xf000) color_data_b[i] = 0xffff;
	}

	/* PseudoColor visual: load this palette */
	if (fb_finfo.visual == FB_VISUAL_PSEUDOCOLOR)
	{
		cmap.start = 0;
		cmap.len = 256;
		cmap.red = color_data_r;
		cmap.green = color_data_g;
		cmap.blue = color_data_b;
		cmap.transp = NULL;
		ioctl(fb_fd, FBIOPUTCMAP, &cmap);
	}
	/* DirectColor visual: load default palette */
	else if (fb_finfo.visual == FB_VISUAL_DIRECTCOLOR)
	{
		u16b palette_r[256], palette_g[256], palette_b[256];

		for (i = 0; i < 256; i++)
		{
			palette_r[i >> (8 - fb_vinfo.red.length)] =
			palette_g[i >> (8 - fb_vinfo.green.length)] =
			palette_b[i >> (8 - fb_vinfo.blue.length)] = (i << 8) | i;
		}

		cmap.start = 0;
		cmap.len = 256;
		cmap.red = palette_r;
		cmap.green = palette_g;
		cmap.blue = palette_b;
		cmap.transp = NULL;

		ioctl(fb_fd, FBIOPUTCMAP, &cmap);
	}
}


/*** Framebuffer initialization/deinitialization ***/

/* Initialize the framebuffer */
void fb_init(void)
{
	int offset;
	const char *errmsg = "Failed to initialize framebuffer!";
	struct fb_cmap cmap;

	/* Open the framebuffer */
	fb_fd = open(fb_name, O_RDWR, 0);
	if (!fb_fd)
	{
		perror("open");
		quit(errmsg);
	}

	/* Get the framebuffer info */
	if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &fb_finfo) < 0)
	{
		perror("ioctl(FBIOGET_FSCREENINFO)");
		quit(errmsg);
	}

	if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &fb_vinfo) < 0)
	{
		perror("ioctl(FBIOGET_VSCREENINFO)");
		quit(errmsg);
	}

	/* Check that we're happy with the visual */
	switch (fb_finfo.visual)
	{
		case FB_VISUAL_PSEUDOCOLOR:
		case FB_VISUAL_DIRECTCOLOR:
		case FB_VISUAL_TRUECOLOR:
			break;
		default:
			quit("Unsupported framebuffer visual");
	}

	/* Memory map fb */
	offset = (((long)fb_finfo.smem_start) -
	         (((long)fb_finfo.smem_start) & ~(PAGE_SIZE - 1)));
	fb_len = fb_finfo.smem_len + offset;
	fb = mmap(NULL, fb_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);

	if (fb == MAP_FAILED)
	{
		perror("mmap");
		quit(errmsg);
	}

	/* We have the palette */
	if (fb_finfo.visual == FB_VISUAL_PSEUDOCOLOR ||
	    fb_finfo.visual == FB_VISUAL_DIRECTCOLOR)
	{
		/* Make sure that the old one is preserved */
		cmap.start = 0;
		cmap.len = 256;
		cmap.red = s_red;
		cmap.green = s_green;
		cmap.blue = s_blue;
		cmap.transp = NULL;
		ioctl(fb_fd, FBIOGETCMAP, &cmap);
	}

	/* Acquire the current mapping */
	tcgetattr(0, &norm_termios);
	tcgetattr(0, &game_termios);

	/*
	 * Turn off canonical mode, echo of input characters, and sending of
	 * SIGTTOU to proc.group of backgr. process writing to controlling terminal
	 */
	game_termios.c_lflag &= ~(ICANON | ECHO | TOSTOP);

	/* Make stdin non-blocking */
	fcntl(0, F_SETFL, O_NONBLOCK | fcntl(0, F_GETFL));

	/* Set terminal parameters */
	tcsetattr(0, TCSAFLUSH, &game_termios);

	/* Turn off the cursor */
	write(1, "\033[?25l", 6);

	/* Initialize the palette */
	Term_xtra_lfb_react();
}


/* Deinitialize the framebuffer */
void fb_quit(void)
{
	struct fb_cmap cmap;

	/* Unmap */
	munmap(fb, fb_len);

	/* Restore palette */
	if (fb_finfo.visual == FB_VISUAL_PSEUDOCOLOR ||
	    fb_finfo.visual == FB_VISUAL_DIRECTCOLOR)
	{
		cmap.start = 0;
		cmap.len = 256;
		cmap.red = s_red;
		cmap.green = s_green;
		cmap.blue = s_blue;
		cmap.transp = NULL;
		ioctl(fb_fd, FBIOPUTCMAP, &cmap);
	}

	/* Close framebuffer device */
	close(fb_fd);

	/* Turn off non-blocking on stdin */
	fcntl(0, F_SETFL, (~O_NONBLOCK) & fcntl(0, F_GETFL));

	/* Reset terminal parameters */
	tcsetattr(0, TCSAFLUSH, &norm_termios);

	/* Clear the screen and turn on the cursor */
	write(1, "\033[2J\033[H", 7);
	write(1, "\033[?25h", 6);
}


/*** Image manipulation ***/

/*
 * Read a font from Win32 BMP file -- adapted from ReadBMP at maid-x11.c
 */
int fb_read_font(char *name, bool quitonerr)
{
	FILE *f;

	BITMAPFILEHEADER fileheader;
	BITMAPINFOHEADER infoheader;

	int ncol;

	size_t total;

	int i;

	u32b y;

	if (font)
	{
		free(font);
		font = NULL;
	}

	/* Open the BMP file */
	f = fopen(name, "r");

	/* No such file */
	if (f == NULL)
	{
		if (quitonerr)
		{
			quit_fmt("Failed to open font file '%s'", name);
		}
		else return (1);
	}

	/* Read the "BITMAPFILEHEADER" */
	rd_u16b(f, &(fileheader.bfType));
	rd_u32b(f, &(fileheader.bfSize));
	rd_u16b(f, &(fileheader.bfReserved1));
	rd_u16b(f, &(fileheader.bfReserved2));
	rd_u32b(f, &(fileheader.bfOffBits));

	/* Read the "BITMAPINFOHEADER" */
	rd_u32b(f, &(infoheader.biSize));
	rd_u32b(f, &(infoheader.biWidth));
	rd_u32b(f, &(infoheader.biHeight));
	rd_u16b(f, &(infoheader.biPlanes));
	rd_u16b(f, &(infoheader.biBitCount));
	rd_u32b(f, &(infoheader.biCompresion));
	rd_u32b(f, &(infoheader.biSizeImage));
	rd_u32b(f, &(infoheader.biXPelsPerMeter));
	rd_u32b(f, &(infoheader.biYPelsPerMeter));
	rd_u32b(f, &(infoheader.biClrUsed));
	rd_u32b(f, &(infoheader.biClrImportand));

	/* Verify the header */
	if (feof(f) ||
	    (fileheader.bfType != 19778) ||
	    (infoheader.biSize != 40))
	{
		if (quitonerr)
		{
			quit_fmt("Incorrect font file format '%s' (not a BMP)", name);
		}
		else return (1);
	}

	/* The two headers above occupy 54 bytes total */
	/* The "bfOffBits" field says where the data starts */
	/* The "biClrUsed" field does not seem to be reliable */
	/* Compute number of colors recorded */
	ncol = (fileheader.bfOffBits - 54) / 4;

	/* Font files must be monochrome */
	if (ncol != 2)
	{
		if (quitonerr)
		{
			quit_fmt("BMP font file '%s' must be monochrome", name);
		}
		else return (1);
	}

	/* Skip the palette (won't need it) */
	for (i = 0; i < ncol; i++)
	{
		u32b dummy;
		rd_u32b(f, &dummy);
	}

	/* Set the character width/height */
	cellw = fontw = infoheader.biWidth / 128;
	cellh = fonth = infoheader.biHeight;

	/* Determine the line length */
	fontpitch = infoheader.biWidth / 8;

	/* Determine total bytes needed for image */
	total = fontpitch * infoheader.biHeight;

	/* Create the pixel buffer */
	font = malloc(total);
	if (!font)
	{
		if (quitonerr)
		{
			quit_fmt("Failed to allocate memory for font data '%s'", name);
		}
		else return (1);
	}

	/* Read the data */
	for (y = 0; y < infoheader.biHeight; y++)
	{
		/* BMPs are upside down. Duh */
		u32b y2 = infoheader.biHeight - y - 1;
		byte *data = font + fontpitch * y2;
		
		/*
		 * I think it's safe to use fread(3) here... we don't care
		 * about endianness ('cause we need just a stream of bits),
		 * and it is in ANSI C.
		 */
		if (fread(data, fontpitch, 1, f) != 1)
		{
			if (quitonerr)
			{
				quit_fmt("Unexpected end of file in '%s'", name);
			}
			else return (1);
		}
	}

	/* We did it! */
	fclose(f);
	return (0);
}


/*
 * Read and prepare tiles from Win32 BMP file.
 */
int fb_read_tiles(char *name, bool quitonerr)
{
	FILE *f;

	BITMAPFILEHEADER fileheader;
	BITMAPINFOHEADER infoheader;

	byte *tmp;

	int ncol;

	size_t total;

	int i;

	u32b x, y;

	char text[50];

	int sc_wid, sc_hgt, x1, y1, x2, y2, Tx, Ty;

	int *px1, *py1, *px2, *py2;
	byte *dx1, *dy1, *dx2, *dy2;

	if (tile)
	{
		free(tile);
		tile = NULL;
	}

	/* Open the BMP file */
	f = fopen(name, "r");

	/* No such file */
	if (f == NULL)
	{
		if (quitonerr)
		{
			quit_fmt("Failed to open tile file '%s'", name);
		}
		else return (1);
	}

	/* Read the "BITMAPFILEHEADER" */
	rd_u16b(f, &(fileheader.bfType));
	rd_u32b(f, &(fileheader.bfSize));
	rd_u16b(f, &(fileheader.bfReserved1));
	rd_u16b(f, &(fileheader.bfReserved2));
	rd_u32b(f, &(fileheader.bfOffBits));

	/* Read the "BITMAPINFOHEADER" */
	rd_u32b(f, &(infoheader.biSize));
	rd_u32b(f, &(infoheader.biWidth));
	rd_u32b(f, &(infoheader.biHeight));
	rd_u16b(f, &(infoheader.biPlanes));
	rd_u16b(f, &(infoheader.biBitCount));
	rd_u32b(f, &(infoheader.biCompresion));
	rd_u32b(f, &(infoheader.biSizeImage));
	rd_u32b(f, &(infoheader.biXPelsPerMeter));
	rd_u32b(f, &(infoheader.biYPelsPerMeter));
	rd_u32b(f, &(infoheader.biClrUsed));
	rd_u32b(f, &(infoheader.biClrImportand));

	/* Verify the header */
	if (feof(f) ||
	    (fileheader.bfType != 19778) ||
	    (infoheader.biSize != 40))
	{
		if (quitonerr)
		{
			quit_fmt("Incorrect tile file format '%s' (not a BMP)", name);
		}
		else return (1);
	}

	/* The two headers above occupy 54 bytes total */
	/* The "bfOffBits" field says where the data starts */
	/* The "biClrUsed" field does not seem to be reliable */
	/* Compute number of colors recorded */
	ncol = (fileheader.bfOffBits - 54) / 4;
	

	for (i = 0; i < ncol; i++)
	{
		RGBQUAD clrg;

		/* Read an "RGBQUAD" */
		rd_byte(f, &(clrg.b));
		rd_byte(f, &(clrg.g));
		rd_byte(f, &(clrg.r));
		rd_byte(f, &(clrg.filler));

		/* 16 colors must be reserved from text.  Ignore them. */
		if (i < 240)
		{
		    	/* Set the palette entry */
		    	angband_color_table[i + 16][1] = clrg.r;
		    	angband_color_table[i + 16][2] = clrg.g;
		    	angband_color_table[i + 16][3] = clrg.b;
		}
	}

	Term_xtra_lfb_react();

	/* Determine total bytes needed for image */
	total = infoheader.biWidth * infoheader.biHeight;

	/* Allocate image memory */
	C_MAKE(tmp, total, byte);

	/* Read the data */
	for (y = 0; y < infoheader.biHeight; y++)
	{
		u32b y2 = infoheader.biHeight - y - 1;

		for (x = 0; x < infoheader.biWidth; x++)
		{
			int ch = getc(f);

			/* Verify not at end of file XXX XXX */
			if (feof(f)) quit_fmt("Unexpected end of file in '%s'", name);

			if (infoheader.biBitCount == 8)
			{
				*(tmp + infoheader.biWidth * y2 + x) = ch + 16;
			}
			else if (infoheader.biBitCount == 4)
			{
				*(tmp + infoheader.biWidth * y2 + x) = (ch / 16) + 16;
				x++;
				*(tmp + infoheader.biWidth * y2 + x) = (ch % 16) + 16;
			}
			else
			{
				if (quitonerr)
				{
					quit_fmt("Illegal color depth %d in '%s' (need 4 or 8)",
					         infoheader.biBitCount, name);
				}
				else return (1);
			}
		}

		/* Make sure user doesn't get bored :) */
		if (!(y % 15))
		{
			strnfmt(text, sizeof(text), "Loading graphics ... %d%%", y * 100 / infoheader.biHeight);
			Term_text_lfb(0, 0, strlen(text), TERM_WHITE, text);
		}
	}

	fclose(f);

	/* Fine!  Now let's resize and convert the image */

	/* Determine the correct tile and array sizes */
	tileh = cellh;
	tilew = cellw * (use_bigtile ? 2 : 1);
	sc_wid = tilew * (infoheader.biWidth / pict_wid);
	sc_hgt = tileh * (infoheader.biHeight / pict_hgt);
	tilepitch = sc_wid;

	/* Allocate the memory */
	tile = malloc(sc_wid * sc_hgt);

	if (pict_wid > tilew)
	{
		px1 = &x1;
		px2 = &x2;
		dx1 = &pict_wid;
		dx2 = &tilew;
	}
	else
	{
		px1 = &x2;
		px2 = &x1;
		dx1 = &tilew;
		dx2 = &pict_wid;
	}

	if (pict_hgt > tileh)
	{
		py1 = &y1;
		py2 = &y2;
		dy1 = &pict_hgt;
		dy2 = &tileh;
	}
	else
	{
		py1 = &y2;
		py2 = &y1;
		dy1 = &tileh;
		dy2 = &pict_hgt;
	}

	Ty = *dy1/2;

	for (y1 = 0, y2 = 0; (y1 < infoheader.biHeight) && (y2 < sc_hgt); )
	{
		Tx = *dx1/2;

		for (x1 = 0, x2 = 0; (x1 < infoheader.biWidth) && (x2 < sc_wid); )
		{
			byte c = *(tmp + infoheader.biWidth * y1 + x1);

			*(tile + tilepitch * (y2) + (x2)) = c;

			(*px1)++;

			Tx -= *dx2;
			if (Tx < 0)
			{
				Tx += *dx1;
				(*px2)++;
			}
		}

		(*py1)++;

		Ty -= *dy2;
		if (Ty < 0)
		{
			Ty += *dy1;
			(*py2)++;
		}

		if (!(y1 % 15))
		{
			strnfmt(text, sizeof(text), "Rescaling graphics ... %d%%", y1 * 100 / infoheader.biHeight);
			Term_text_lfb(0, 0, strlen(text), TERM_WHITE, text);
		}
	}

	/* Success.  Free the raw tiles array */
	free(tmp);

	return (0);
}


/* Nothing exciting here, we support only a single window */
typedef struct term_data term_data;

struct term_data
{
	term t;
};


static term_data data;


/*** Function hooks needed by "Term" ***/


/*
 * Init a new "term" (the actual work is done by fb_init())
 */
static void Term_init_lfb(term *t)
{
}


/*
 * Nuke an old "term" (the actual work is done by fb_quit())
 */
static void Term_nuke_lfb(term *t)
{
	fb_quit();
}



/*
 * Do a "user action" on the current "term"
 * Implements "user interface settings" menu
 */
static errr Term_user_lfb(int n)
{
	term *t = &data.t;
	char key, buf[80], path[250], *visual, *graf;

	while (1)
	{
		/*
		 * Print a bunch of information and a menu
		 */
	    
		screen_save();

		prt(" Linux framebuffer display module for Angband, v." LFB_VERSION, 1, 20);
		prt(" Copyright (C) 2005 A. Ulyanov <posband@earthsea.org>", 2, 20);

		prt("", 3, 20);

		switch (fb_finfo.visual)
		{
			case FB_VISUAL_PSEUDOCOLOR: visual = "PseudoColor"; break;
			case FB_VISUAL_DIRECTCOLOR: visual = "DirectColor"; break;
			case FB_VISUAL_TRUECOLOR:   visual = "TrueColor"; break;
			default:                    visual = "Unknown!"; break;
		}

		strnfmt(buf, sizeof(buf), " Running on: %s   Visual: %s", fb_finfo.id, visual);
		prt(buf, 4, 20);
		strnfmt(buf, sizeof(buf), " %dx%dx%dbpp  (%dx%d window)",
		        fb_vinfo.xres, fb_vinfo.yres, fb_vinfo.bits_per_pixel,
		        t->wid, t->hgt);
		prt(buf, 5, 20);

		prt("", 6, 20);

		switch (use_graphics)
		{
			case GRAPHICS_NONE:		graf = "None"; break;
			case GRAPHICS_ORIGINAL:		graf = "Original (8x8)"; break;
			case GRAPHICS_ADAM_BOLT:	graf = "Adam Bolt (16x16)"; break;
			case GRAPHICS_DAVID_GERVAIS:	graf = "David Gervais (32x32)"; break;
			default:			graf = "Other"; break;
		}

		strnfmt(buf, sizeof(buf), " Font: %s   Graphics: %s", fb_font, graf);
		prt(buf, 7, 20);

		strnfmt(buf, sizeof(buf), " Current tile size: %dx%d%s", cellw, cellh,
		        use_bigtile ? " (double-width mode)" : "");
		prt(buf, 8, 20);

		prt("", 9, 20);

		prt("  a) Change font", 10, 20);
		prt("  b) Change graphics", 11, 20);
		prt("  c) Switch bigtile mode", 12, 20);
		prt("  d) Decrease tile width", 13, 20);
		prt("  e) Decrease tile height", 14, 20);
		prt("  f) Increase tile width", 15, 20);
		prt("  g) Increase tile height", 16, 20);

		prt("", 17, 20);

		prt("  ESC) Exit", 18, 20);

		prt("", 19, 20);

		key = tolower(inkey());

		screen_load();

		if (key == ESCAPE) break;	

		switch (key)
		{
			/* Change font */
			case 'a':
			{
				strcpy(buf, fb_font);
				if (!get_string("Font: ", buf, sizeof(buf))) return (0);

				path_build(path, sizeof(path), ANGBAND_DIR_XTRA_FONT, buf);
				strcat(path, ".bmp");

				/*
				 * Try to read the font specified.  If this fails, reload
				 * the old font (if this fails too, panic).
				 */
				if (fb_read_font(path, FALSE))
				{
					path_build(path, sizeof(path), ANGBAND_DIR_XTRA_FONT, fb_font);
					strcat(path, ".bmp");

					if (fb_read_font(path, FALSE)) exit_game_panic();

					msg_print("Failed to load font!");
				}
				else
				{
					/* Hack -- make sure that the font isn't too large */
					if (fb_vinfo.xres / fontw < 80 || fb_vinfo.yres / fonth < 24)
					{
						path_build(path, sizeof(path), ANGBAND_DIR_XTRA_FONT, fb_font);
						strcat(path, ".bmp");

						if (fb_read_font(path, FALSE)) exit_game_panic();

						msg_print("Font is too large for this resolution!");

						break;
					}

					/* Set the new font name */
					fb_font = string_make(buf);

					/* Reload the tiles */
					if (use_graphics)
					{
						path_build(path, sizeof(path), ANGBAND_DIR_XTRA_GRAF, fb_tile);
						fb_read_tiles(path, TRUE);
					}

					/* Resize the terminal */
					Term_resize((fb_vinfo.xres / cellw >= 256 ? 255 : fb_vinfo.xres / cellw),
					            (fb_vinfo.yres / cellh >= 256 ? 255 : fb_vinfo.yres / cellh));

					/* Refresh it */
					do_cmd_redraw();
				}
				
			}
			break;

			/* Change graphics */
			case 'b':
			{
				screen_save();

				prt(" Select the tileset:", 1, 20);
				prt("", 2, 20);
				prt("  a) None", 3, 20);
				prt("  b) Original (8x8)", 4, 20);
				prt("  c) Adam Bolt (16x16)", 5, 20);
				prt("  d) David Gervais (32x32)", 6, 20);
				prt("", 7, 20);

				key = tolower(inkey());

				screen_load();

				switch (key)
				{
					/* No graphics */
					case 'a':
					{
						use_graphics = GRAPHICS_NONE;
						use_transparency = FALSE;
						free(tile);
						tile = NULL;
						ANGBAND_GRAF = "";
						t->higher_pict = FALSE;
					}
					break;

					/* Old graphics */
					case 'b':
					{
						use_graphics = GRAPHICS_ORIGINAL;
						use_transparency = FALSE;
						pict_wid = pict_hgt = 8;
						ANGBAND_GRAF = "old";
						t->higher_pict = TRUE;

						fb_tile = "8X8.bmp";
						path_build(path, sizeof(path), ANGBAND_DIR_XTRA_GRAF, fb_tile);
						if (fb_read_tiles(path, FALSE))
						{
							/* Turn off graphics on failure */
							msg_print("Failed to load tiles!");
							use_graphics = GRAPHICS_NONE;
							use_transparency = FALSE;
							ANGBAND_GRAF = "";
							t->higher_pict = FALSE;
						}
					}
					break;

					/* A. Bolt's graphics */
					case 'c':
					{
						use_graphics = GRAPHICS_ADAM_BOLT;
						use_transparency = TRUE;
						pict_wid = pict_hgt = 16;
						ANGBAND_GRAF = "new";
						t->higher_pict = TRUE;

						fb_tile = "16x16.bmp";
						path_build(path, sizeof(path), ANGBAND_DIR_XTRA_GRAF, fb_tile);
						if (fb_read_tiles(path, FALSE))
						{
							/* Turn off graphics on failure */
							msg_print("Failed to load tiles!");
							use_graphics = GRAPHICS_NONE;
							use_transparency = FALSE;
							ANGBAND_GRAF = "";
							t->higher_pict = FALSE;
						}
					}
					break;

					/* D. Gervais' graphics */
					case 'd':
					{
						use_graphics = GRAPHICS_DAVID_GERVAIS;
						use_transparency = TRUE;
						pict_wid = pict_hgt = 32;
						ANGBAND_GRAF = "david";
						t->higher_pict = TRUE;

						fb_tile = "32x32.bmp";
						path_build(path, sizeof(path), ANGBAND_DIR_XTRA_GRAF, fb_tile);
						if (fb_read_tiles(path, FALSE))
						{
							/* Turn off graphics on failure */
							msg_print("Failed to load tiles!");
							use_graphics = GRAPHICS_NONE;
							use_transparency = FALSE;
							ANGBAND_GRAF = "";
							t->higher_pict = FALSE;
						}
					}
					break;
				}

				/* Reset visuals and force full refresh */
				reset_visuals(TRUE);
				do_cmd_redraw();
			}
			break;

			/* Switch bigtile mode */
			case 'c':
			{
				use_bigtile = !use_bigtile;

				/* Reload graphics if necessary */
				if (use_graphics)
				{
					path_build(path, sizeof(path), ANGBAND_DIR_XTRA_GRAF, fb_tile);
					fb_read_tiles(path, TRUE);
				}

				do_cmd_redraw();
			}
			break;

			/* Change cell size */
			case 'd':
			{
				if (cellw > fontw)
				{
					cellw--;

					/* Reload the tiles */
					if (use_graphics)
					{
						path_build(path, sizeof(path), ANGBAND_DIR_XTRA_GRAF, fb_tile);
						fb_read_tiles(path, TRUE);
					}

					/* Resize the terminal */
					Term_resize((fb_vinfo.xres / cellw >= 256 ? 255 : fb_vinfo.xres / cellw),
					            (fb_vinfo.yres / cellh >= 256 ? 255 : fb_vinfo.yres / cellh));

					/* Refresh it */
					do_cmd_redraw();
				}
			}
			break;

			case 'e':
			{
				if (cellh > fonth)
				{
					cellh--;

					/* Reload the tiles */
					if (use_graphics)
					{
						path_build(path, sizeof(path), ANGBAND_DIR_XTRA_GRAF, fb_tile);
						fb_read_tiles(path, TRUE);
					}

					/* Resize the terminal */
					Term_resize((fb_vinfo.xres / cellw >= 256 ? 255 : fb_vinfo.xres / cellw),
					            (fb_vinfo.yres / cellh >= 256 ? 255 : fb_vinfo.yres / cellh));

					/* Refresh it */
					do_cmd_redraw();
				}
			}
			break;

			case 'f':
			{
				if (cellw < 64)
				{
					cellw++;

					/* Reload the tiles */
					if (use_graphics)
					{
						path_build(path, sizeof(path), ANGBAND_DIR_XTRA_GRAF, fb_tile);
						fb_read_tiles(path, TRUE);
					}

					/* Resize the terminal */
					Term_resize((fb_vinfo.xres / cellw >= 256 ? 255 : fb_vinfo.xres / cellw),
					            (fb_vinfo.yres / cellh >= 256 ? 255 : fb_vinfo.yres / cellh));

					/* Refresh it */
					do_cmd_redraw();
				}
			}
			break;

			case 'g':
			{
				if (cellh < 64)
				{
					cellh++;

					/* Reload the tiles */
					if (use_graphics)
					{
						path_build(path, sizeof(path), ANGBAND_DIR_XTRA_GRAF, fb_tile);
						fb_read_tiles(path, TRUE);
					}

					/* Resize the terminal */
					Term_resize((fb_vinfo.xres / cellw >= 256 ? 255 : fb_vinfo.xres / cellw),
					            (fb_vinfo.yres / cellh >= 256 ? 255 : fb_vinfo.yres / cellh));

					/* Refresh it */
					do_cmd_redraw();
				}
			}
			break;
		}
	}

	/* Success */
	return (0);
}


/*
 * Do a "special thing" to the current "term"
 */
static errr Term_xtra_lfb(int n, int v)
{
	/* Analyze */
	switch (n)
	{
		case TERM_XTRA_EVENT:
		{
			int lch;
			unsigned char ch;
			fd_set s;

			if (v)
			{
				/* Wait for input */
				FD_ZERO(&s);
				FD_SET(0, &s);
				select(1, &s, NULL, NULL, NULL);
			}

			lch = -1;

			/* Loop while reading a character */
			while (read(0, &ch, 1) == 1)
			{
				/* Consume previous character, if any */
				if (lch != -1) Term_keypress(lch);

				lch = ch;
			}

			/* Consume last character */
			if (lch != -1)
			{
				if (lch == ESCAPE)
				{
					/* util.c treats '`' as ESCAPE, but without waiting   *
					 * for later characters in case ESCAPE starts a macro */
					Term_keypress('`');
				}
				else
				{
					Term_keypress(lch);
				}
			}

			return (0);
		}

		case TERM_XTRA_FLUSH:
		{
			/* Flush input */
			unsigned char ch;
			while (read(0, &ch, 1) == 1);
			return (0);
		}

		case TERM_XTRA_CLEAR:
		{
			/*
			 * XXX Hardcoded sequence.  Should be okay
			 * (we're going to use only Linux console)
			 */
			write(1, "\033[2J\033[H", 7);

			return (0);
		}

		case TERM_XTRA_NOISE:
		{
			/* ASCII BEL */
			write(1, "\007", 1);

			return (0);
		}


		case TERM_XTRA_REACT:
		{
			Term_xtra_lfb_react();
			return (0);
		}

		case TERM_XTRA_ALIVE:
		{
			/* Change the "hard" level */
			if (!v)
			{
				fb_quit();
			}
			else
			{
				fb_init();
			}

			return (0);
		}

		case TERM_XTRA_DELAY:
		{
			/* Delay with usleep() */
			if (v > 0) usleep(v * 1000);

			return (0);
		}
	}

	/* Unknown or Unhandled action */
	return (1);
}


/*
 * Display the cursor
 */
static errr Term_curs_lfb(int x, int y)
{
	int i;
	int x1 = x * cellw, x2 = (x + 1) * cellw;
	int y1 = y * cellh, y2 = (y + 1) * cellh;

	/* Paranoia */
	if (!font) return (1);

	/* Draw a "gray box" cursor (similar to GUI front-ends) */
	for (i = x1; i < x2; i++) putpixel(i, y1, TERM_SLATE);
	for (i = y1; i < y2; i++) putpixel(x1, i, TERM_SLATE);
	for (i = x1; i < x2; i++) putpixel(i, y2 - 1, TERM_SLATE);
	for (i = y1; i < y2; i++) putpixel(x2 - 1, i, TERM_SLATE);

	/* Success */
	return (0);
}

static errr Term_bigcurs_lfb(int x, int y)
{
	int i;
	int x1 = x * cellw, x2 = (x + 2) * cellw;
	int y1 = y * cellh, y2 = (y + 1) * cellh;

	/* Paranoia */
	if (!font) return (1);

	/* Draw a "gray box" cursor (similar to GUI front-ends) */
	for (i = x1; i < x2; i++) putpixel(i, y1, TERM_SLATE);
	for (i = y1; i < y2; i++) putpixel(x1, i, TERM_SLATE);
	for (i = x1; i < x2; i++) putpixel(i, y2 - 1, TERM_SLATE);
	for (i = y1; i < y2; i++) putpixel(x2 - 1, i, TERM_SLATE);

	/* Success */
	return (0);
}


/*
 * Erase some characters
 */
static errr Term_wipe_lfb(int x, int y, int n)
{
	int i, j;
	int x1 = x * cellw, x2 = (x + n) * cellw;
	int y1 = y * cellh, y2 = (y + 1) * cellh;
	
	/* Paranoia */
	if (!font) return (1);

	/* Hack -- simple way */
	for (i = x1; i < x2; i++)
	{
		for (j = y1; j < y2; j++)
		{
			putpixel(i, j, 0);
		}
	}

	/* Success */
	return (0);
}


/*
 * Draw some text on the screen
  */
static errr Term_text_lfb(int x, int y, int n, byte a, const char *cp)
{
	int i, j, k;
	int y1 = y * cellh;

	/* Paranoia */
	if (!font) return (1);

	/* Hack -- first blank the entire area, if the cell is not equal to character */
	if (cellw != fontw || cellh != fonth) Term_wipe_lfb(x, y, n);

	/* XXX Perhaps could be optimized... */

	for (k = 0; k < n; k++)
	{
		int x1 = (x + k) * cellw, x2 = (*cp++ & 0x7f) * fontw;

		for (i = 0; i < fontw; i++)
		{
			for (j = 0; j < fonth; j++)
			{
				putpixel(i + x1 + (cellw - fontw) / 2 , j + y1 + (cellh - fonth) / 2, 
				         (font[j * fontpitch + (x2 + i) / 8] & (0x80 >> ((x2 + i) % 8)) ? (a & 0x0f) : 0));
			}
		}
	}

	/* Success */
	return (0);
}


/*
 * Draw some attr/char pairs on the screen
 */
static errr Term_pict_lfb(int x, int y, int n, const byte *ap, const char *cp,
                          const byte *tap, const char *tcp)
{
	int i, j, k;
	int y1 = y * cellh;
	byte blank;

	/* Mega-hack -- "blank" pixel */
	if (use_graphics == GRAPHICS_DAVID_GERVAIS)
	{
		blank = *tile;
	}
	else
	{
		blank = *(tile + 6 * tilepitch * tileh);
	}

	/* Paranoia */
	if (!tile) return (1);

	/* XXX Perhaps could be optimized... */

	for (k = 0; k < n; k++, tcp++, tap++, cp++, ap++)
	{
		int x1 = x * cellw + k * tilew;
		int x2 = (*tcp & 0x7f) * tilew;
		int y2 = (*tap & 0x7f) * tileh;

		for (i = 0; i < tilew; i++)
		{
			for (j = 0; j < tileh; j++)
			{
				putpixel(i + x1, j + y1, (tile[(y2 + j) * tilepitch + x2 + i]));
			}
		}

		if (*cp != *tcp || *ap != *tap)
		{
			x2 = (*cp & 0x7f) * tilew;
			y2 = (*ap & 0x7f) * tileh;

			for (i = 0; i < tilew; i++)
			{
				for (j = 0; j < tileh; j++)
				{
					byte pixel = tile[(y2 + j) * tilepitch + x2 + i];

					if (pixel != blank) putpixel(i + x1, j + y1, pixel);
				}
			}
		}
	}

	/* Success */
	return (0);
}





/*
 * Help message.
 *   1st line = max 68 chars.
 *   Start next lines with 11 spaces, as in main-xaw.c.
 */
const char help_lfb[] = "Linux framebuffer, subopts -d<device> -f<font> -c<wid>x<hgt>"
#ifdef USE_GRAPHICS
			"\n           -b (bigtile) -o (original) -a (Adam Bolt) -g (David Gervais)"
#endif
			;


/*
 * Initialization function
 */
errr init_lfb(int argc, char **argv)
{
	term *t = &data.t;
	char path[250], *tty;
	int i, cx = 0, cy = 0;

	/* Check the arguments */
	for (i = 1; i < argc; i++)
	{
		if (prefix(argv[i], "-d"))
			fb_name = &argv[i][2];
		if (prefix(argv[i], "-f"))
			fb_font = string_make(&argv[i][2]);
		if (prefix(argv[i], "-c"))
			sscanf(&argv[i][2], "%dx%d", &cx, &cy);
#ifdef USE_GRAPHICS
		if (prefix(argv[i], "-o"))
			arg_graphics = GRAPHICS_ORIGINAL;
		if (prefix(argv[i], "-a"))
			arg_graphics = GRAPHICS_ADAM_BOLT;
		if (prefix(argv[i], "-g"))
			arg_graphics = GRAPHICS_DAVID_GERVAIS;
		if (prefix(argv[i], "-b"))
			use_bigtile = TRUE;
#endif /* USE_GRAPHICS */
	}

	/* Check that we run at the VT */
	tty = ttyname(0);
	if (!tty || (!prefix(tty, "/dev/tty") && !prefix(tty, "/dev/vc/")))
	{
		quit_fmt("Must be run on the virtual terminal (tty is %s)", tty);
	}

	/* Build the "font" path */
	path_build(path, sizeof(path), ANGBAND_DIR_XTRA, "font");

	/* Allocate the path */
	ANGBAND_DIR_XTRA_FONT = string_make(path);

	/* Load the font */
	path_build(path, sizeof(path), ANGBAND_DIR_XTRA_FONT, fb_font);
	strcat(path, ".bmp");
	fb_read_font(path, TRUE);

	/* Preserve the cell dimensions */
	if (cx && cy)
	{
		cellw = cx;
		cellh = cy;
	}

	/* Initialize the framebuffer */
	fb_init();

	/* Initialize the term */
	term_init(t, (fb_vinfo.xres / cellw >= 256 ? 255 : fb_vinfo.xres / cellw),
		     (fb_vinfo.yres / cellh >= 256 ? 255 : fb_vinfo.yres / cellh), 256);

#ifdef USE_GRAPHICS
	/* Build the "graphics" path */
	path_build(path, sizeof(path), ANGBAND_DIR_XTRA, "graf");

	/* Allocate the path */
	ANGBAND_DIR_XTRA_GRAF = string_make(path);

	/* Initialize graphics */
	switch (arg_graphics)
	{
		case GRAPHICS_DAVID_GERVAIS:
		{
			fb_tile = "32x32.bmp";
			path_build(path, sizeof(path), ANGBAND_DIR_XTRA_GRAF, fb_tile);

			/* Check whether the file is here */
			if (0 == fd_close(fd_open(path, O_RDONLY)))
			{
				/* Use graphics */
				use_graphics = GRAPHICS_DAVID_GERVAIS;
				use_transparency = TRUE;
				pict_wid = pict_hgt = 32;
				ANGBAND_GRAF = "david";
				t->higher_pict = TRUE;
				break;
			}
			/* Fall through */
		}

		case GRAPHICS_ADAM_BOLT:
		{
			fb_tile = "16x16.bmp";
			path_build(path, sizeof(path), ANGBAND_DIR_XTRA_GRAF, fb_tile);

			/* Check whether the file is here */
			if (0 == fd_close(fd_open(path, O_RDONLY)))
			{
				/* Use graphics */
				use_graphics = GRAPHICS_ADAM_BOLT;
				use_transparency = TRUE;
				pict_wid = pict_hgt = 16;
				ANGBAND_GRAF = "new";
				t->higher_pict = TRUE;
				break;
			}
			/* Fall through */
		}

		case GRAPHICS_ORIGINAL:
		{
			fb_tile = "8X8.bmp";
			path_build(path, sizeof(path), ANGBAND_DIR_XTRA_GRAF, fb_tile);

			/* Check whether the file is here */
			if (0 == fd_close(fd_open(path, O_RDONLY)))
			{
				/* Use graphics */
				use_graphics = GRAPHICS_ORIGINAL;
				pict_wid = pict_hgt = 8;
				ANGBAND_GRAF = "old";
				t->higher_pict = TRUE;
				break;
			}
			break;
		}
	}
#endif /* USE_GRAPHICS */

	/* Choose "soft" or "hard" cursor XXX XXX XXX */
	t->soft_cursor = TRUE;

	/* Ignore the "TERM_XTRA_BORED" action XXX XXX XXX */
	t->never_bored = TRUE;

	/* Ignore the "TERM_XTRA_FROSH" action XXX XXX XXX */
	t->never_frosh = TRUE;

	/* Erase with "white space" */
	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';

	/* Prepare the init/nuke hooks */
	t->init_hook = Term_init_lfb;
	t->nuke_hook = Term_nuke_lfb;

	/* Prepare the template hooks */
	t->user_hook = Term_user_lfb;
	t->xtra_hook = Term_xtra_lfb;
	t->curs_hook = Term_curs_lfb;
	t->bigcurs_hook = Term_bigcurs_lfb;
	t->wipe_hook = Term_wipe_lfb;
	t->text_hook = Term_text_lfb;
	t->pict_hook = Term_pict_lfb;

	/* Remember where we came from */
	t->data = &data;

	/* Activate it */
	Term_activate(t);

#ifdef USE_GRAPHICS
	/* Load graphics, if necessary */
	if (use_graphics)
	{
		Term_xtra_lfb(TERM_XTRA_CLEAR, 0);

		fb_read_tiles(path, TRUE);
	}
#endif /* USE_GRAPHICS */

	/* Global pointer */
	angband_term[0] = t;

	/* Success */
	return (0);
}

#endif /* USE_LFB */
