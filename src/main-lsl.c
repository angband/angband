/*****************************************************************************
 * File: main-lsl.c
 * Purpose: Support for Linux-SVGALIB Angband
 * Original Author: Jon Taylor (taylorj@gaia.ecs.csus.edu)
 * Update by: Dennis Payne (dulsi@identicalsoftware.com)
 * Version: 1.4.0, 12/05/99
 *
 * Some of the ideas in this code came from main-win.c by Skirmantas Kligys
 * (kligys@scf.usc.edu).
 ****************************************************************************/

/* Angband header files */
#include "angband.h"

#ifdef USE_LSL

#include "main.h"

/* Standard C header files */
#include <stdio.h>
#include <stdlib.h>

/* SVGAlib header files */
#include <vga.h>
#include <vgagl.h>
#include <vgakeyboard.h>
#include <zlib.h>

static cptr ANGBAND_DIR_XTRA_GRAF;

#define COLOR_OFFSET 240

/* Define font/graphics cell width and height */
#define CHAR_W 8
#define CHAR_H 13

/*****************************************************************************
 * .BMP file handling stuff.  This needs to get reorganized someday....
 ****************************************************************************/

/* BITMAPFILEHEADER
 *
 * Bitmap File Information
 *
 * The BITMAPFILEHEADER data structure contains information about the type,
 * size, and layout of a device-independent bitmap (DIB) file.
 */
typedef struct BITMAPFILEHEADER {
	short   bfType;
	int     bfSize;
	short   bfReserved1;
	short   bfReserved2;
	int     bfOffBits;
} BITMAPFILEHEADER;

typedef struct BITMAPINFOHEADER {
	unsigned int  biSize;
	unsigned int  biWidth;
	unsigned int  biHeight;
	unsigned short   biPlanes;
	unsigned short   biBitCount;
	unsigned int  biCompression;
	unsigned int  biSizeImage;
	unsigned int  biXPelsPerMeter;
	unsigned int  biYPelsPerMeter;
	unsigned int  biClrUsed;
	unsigned int  biClrImportant;
} BITMAPINFOHEADER;

typedef struct BITMAPCOREHEADER {
	unsigned int  bcSize;
	unsigned short  bcWidth;
	unsigned short  bcHeight;
	unsigned short   bcPlanes;
	unsigned short   bcBitCount;
} BITMAPCOREHEADER;

typedef struct {
	int width,height,bpp,numcols;
	char type[4];
} PICINFO;

static int flip(unsigned char *image, unsigned int w, unsigned int h);

/*
 * Image palette, width, and height should be arguments but quickly hacked
 * in  - Dennis */
static unsigned char *pal;
static unsigned int bw, bh;

void *read_bmp_file(void)
{
	FILE *infile;
	int i, j;
	int iswindows = 0;
	int dummy, count, done;
	int output_type = 0;
	unsigned char *buf, *bmap;
	unsigned char read[2];
	unsigned char *p, *ptr, *dptr, *hptr;
	unsigned int w, h, palsize;
	char path[255];
	BITMAPINFOHEADER bih;
	BITMAPCOREHEADER bch;
	PICINFO *pp;

	pp = malloc(sizeof(PICINFO));

	bmap = NULL;
	pal = NULL;

	sprintf(path, "%s/8x13.bmp", ANGBAND_DIR_XTRA_GRAF);
	if (!(infile = fopen(path, "r")))
	{
		printf("Unable to load bitmap data file %s, bailing out....\n", path);
 		exit(-1);
 	}

	buf = (unsigned char *)malloc(54);
	fread(buf, 1, 26, infile);
	memcpy(&bch, buf + 14, 12);

	/* Truly MS_Windows 3.x ? */
	if (bch.bcSize == 40)
	{
		fread(buf + 26, 1, 28, infile); /* Then we need the rest */
		memcpy(&bih, buf + 14, 40);
		iswindows = TRUE;
	}
	else
	{
		iswindows = FALSE;
	}

	p = malloc(768);
	pal = p;

	if (iswindows)
	{
		/* MS_Windows 3.x */
		pp->width = w = bih.biWidth;
		pp->height = h = bih.biHeight;
		pp->bpp = bih.biBitCount;

		/*
		 * Here the "offbits" -  we need 'em for the
		 * palette size - e.g. XV uses different sizes
		 */
		palsize = (*(unsigned *) (buf + 10) - 54) / 4;
	}
	else
	{
		/* OS/2 V1.1 */
		pp->width = w = bch.bcWidth;
		pp->height = h = bch.bcHeight;
		pp->bpp = bch.bcBitCount;
		palsize = (*(unsigned *) (buf + 10) - 26) / 3;
	}

	bw = pp->width;
	bh = pp->height;

	if ((pp->bpp >> 3) < 3) output_type = 1;

	/* w is the size of a horizontal line in bytes
	 * bih.biWidth is the number of valid pixels in a line
	 * the latter one is passed to vgadisp.c as pp->width
	 */
	switch (pp->bpp)
	{
	case 1:
		if (w % 32)
			w = (w / 32) * 32 + 32;
		break;
	case 4:
		if (w % 8)
			w = (w / 8) * 8 + 8;
		break;
	case 8:
		if (w % 4)
			w = (w / 4) * 4 + 4;
		break;
	}

	if ((pp->bpp == 24) && (output_type == 3)) dummy = 3;
	else dummy = 1;

	bmap = malloc(w * (h + 2) * dummy);
	memset(bmap, 0, w * (h + 2) * dummy);

	switch (pp->bpp)
	{
	case 1:
		/* 1bit non compressed */
		ptr = pal;
		fread(ptr, 1, 3, infile);

		if (iswindows)
			fread(&dummy, 1, 1, infile);

		dummy = ptr[0];
		ptr[0] = ptr[2] / 4;
		ptr[1] /= 4;
		ptr[2] = dummy / 4;
		fread(ptr + 3, 1, 3, infile);

		if (iswindows)
			fread(&dummy, 1, 1, infile);

		dummy = ptr[3];
		ptr[3] = ptr[5] / 4;
		ptr[4] /= 4;
		ptr[5] = dummy / 4;
		ptr = bmap;

		for (j = h - 1; j >= 0; j--)
		{
			for (i = 0, count = 0 ; i < (w >> 3); i++)
			{
				hptr = ptr + j * pp->width;
				dummy = fgetc(infile);
				if (count < pp->width)
				{
					hptr[count] = (dummy & 128) ? 1 : 0; count++;
					hptr[count] = (dummy & 64) ? 1 : 0; count++;
					hptr[count] = (dummy & 32) ? 1 : 0; count++;
					hptr[count] = (dummy & 16) ? 1 : 0; count++;
					hptr[count] = (dummy & 8) ? 1 : 0; count++;
					hptr[count] = (dummy & 4) ? 1 : 0; count++;
					hptr[count] = (dummy & 2) ? 1 : 0; count++;
					hptr[count] = dummy & 1; count++;
				}
			}
		}

		pp->numcols = 2;
		break;

	case 4:
		/* 4bit non compressed */
		ptr = pal;

		for (i = 0; i < palsize; i++)
		{
			fread(ptr + 3 * i, 1, 3, infile);
			if (iswindows)
				fread(&dummy, 1, 1, infile);
			dummy = ptr[3 * i];
			ptr[3 * i] = ptr[3 * i + 2] / 4;
			ptr[3 * i + 1] /= 4;
			ptr[3 * i + 2] = dummy / 4;
		}

		ptr = bmap;

		if ((!iswindows) || (bih.biCompression == 0))
		{
			for (j = h - 1; j >= 0; j--)
			{
				for (i = 0, count = 0; i < (w / 2); i++)
				{
					dummy = fgetc(infile);
					if (count < pp->width)
					{
						ptr[count + j * pp->width] = dummy >> 4;
						count++;
					}
					if (count < pp->width)
					{
						ptr[count + j * pp->width] = dummy & 15;
						count++;
					}
				}
			}
		}
		else
		{
			/* 4bit RLE compressed */
			done = 0;
			count = 0;

			while (done == 0)
			{
				fread(read, 1, 2, infile);

				if (*read)
				{
					i = 0;

					do
					{
						*ptr = read[1] >> 4;
						ptr++;
						i++;

						if (i < (read[0]))
						{
							*ptr = read[1] & 15;
							ptr++;
							i++;
						}
					}
					while (i < (*read));
				}
				else if (read[1] == 0)
				{
					count++;
				}
				else if (read[1] == 1)
					done = 1;
				else if (read[1] == 2)
				{
					/* This isn't really tested */
					ptr += fgetc(infile) + bih.biWidth * fgetc(infile);
				}
				else
				{
					dptr = hptr = (unsigned char *) malloc(read[1] >> 1);
					fread(dptr, 1, read[1] >> 1, infile);

					if (read[1] % 4 > 1)
						dummy = fgetc(infile);

					i = 0;

					do
					{
						*ptr = (*dptr) >> 4;
						i++;
						ptr++;

						if (i < read[1])
						{
							*ptr = (*dptr) & 15;
							i++;
							ptr++;
						}

						dptr++;
					}
					while (i < read[1]);

					free(hptr);
				}
			}

			flip(bmap, bih.biWidth, bih.biHeight);
		}

		pp->width = w;
		pp->numcols= 16;
		break;

	case 8:
		/* 8bit non compressed */
		ptr = pal;

		for (i = 0; i < palsize; i++)
		{
			fread(ptr + 3 * i, 1, 3, infile);

			if (iswindows)
				dummy = fgetc(infile);

			dummy = ptr[3 * i];
			ptr[3 * i] = ptr[3 * i + 2] / 4;
			ptr[3 * i + 1] /= 4;
			ptr[3 * i + 2] = dummy / 4;
		}

		ptr = bmap;

		if ((!iswindows) || (bih.biCompression == 0))
		{
			for (i = h - 1; i >= 0; i--)
			{
				fread(ptr + pp->width * i, 1, w, infile);
			}
		}
		else
		{
			/* 8bit RLE compressed */
			done = 0;
			count = 0;

			while (done == 0)
			{
				fread(read, 1, 2, infile);
				if (read[0])
				{
					for (i = 0; i < (int)read[0]; i++)
					{
						*ptr = read[1];
						ptr++;
					}
				}
				else if (read[1] == 0)
				{
					count++;
				}
				else if (read[1] == 1)
					done = 1;
				else if (read[1] == 2)
					ptr += fgetc(infile) + bih.biWidth * fgetc(infile);
				else
				{
					fread(ptr, 1, read[1], infile);
					if (read[1] % 2)
						fgetc(infile);
					ptr += read[1];
				}
			}

			flip(bmap, bih.biWidth, bih.biHeight);
		}

		pp->numcols = 256;

		break;
	}

	free(buf);
	fclose(infile);
	return (bmap);
}


static int flip(unsigned char *image, unsigned int w, unsigned int h)
{
	unsigned int i;
	unsigned char *hptr;

	hptr = (unsigned char *)malloc(w);

	for (i = 0; i < (h / 2); i++)
	{
		memcpy(hptr, image + i * w, w);
		memcpy(image + i * w, image + (h - i - 1) * w, w);
		memcpy(image + (h - i - 1) * w, hptr, w);
	}

	free(hptr);

	return (0);
}


/* ======================================================================= */

/* The main "term" structure */
static term term_screen_body;

/* The visible and virtual screens */
GraphicsContext *screen;
GraphicsContext *backscreen;
GraphicsContext *buffer;

/* The font data */
void *font;


/* Initialize the screen font */
void initfont(void)
{
	gzFile fontfile;
	void *temp;
	long junk;

	if (!(fontfile = gzopen("/usr/lib/kbd/consolefonts/lat1-12.psfu.gz", "r")))
	{
		/* Try uncompressed */
		if (!(fontfile = gzopen("/usr/lib/kbd/consolefonts/lat1-12.psfu", "r")))
		{
			printf("Error: could not open font file.  Aborting....\n");
			exit(1);
		}
	}

	/* Junk the 4-byte header */
	gzread(fontfile, &junk, 4);

	/* Initialize font */
	/* Haven't looked into why it is 3328 - Dennis */
	temp = malloc(/*12*256*/ 3328);
	gzread(fontfile, temp, 3328);
	font = malloc(256 * 8 * 12 * BYTESPERPIXEL);
	gl_expandfont(8, 12, 15, temp, font);
	gl_setfont(8, 12, font);
	free(temp);
	gzclose(fontfile);
}


/* Initialize palette values for colors 0-15 */
void setpal(void)
{
	int i;

	gl_setpalette(pal);

	for (i = 0; i < 16; i++)
	{
		gl_setpalettecolor(COLOR_OFFSET + i, angband_color_table[i][1] >> 2,
			angband_color_table[i][2] >> 2, angband_color_table[i][3] >> 2);
	}

#if 0
	gl_setpalettecolor (0,00,00,00); /* Black */
	gl_setpalettecolor (3,63,63,63); /* White */
	gl_setpalettecolor (13,40,40,40); /* Gray */
	gl_setpalettecolor (11,25,15,05); /* Orange */
	gl_setpalettecolor (2,32,00,00); /* Red */
	gl_setpalettecolor (7,00,32,00); /* Green */
	gl_setpalettecolor (12,00,00,40); /* Blue */
	gl_setpalettecolor (5,50,25,00); /* Brown */
	gl_setpalettecolor (1,28,28,28); /* Dark Gray */
	gl_setpalettecolor (10,52,52,52); /* Light Gray */
	gl_setpalettecolor (15,41,00,63); /* Purple */
	gl_setpalettecolor (4,63,63,00); /* Yellow */
	gl_setpalettecolor (14,63,00,00); /* Light Red */
	gl_setpalettecolor (6,00,63,00); /* Light Green */
	gl_setpalettecolor (9,00,50,63); /* Light Blue */
	gl_setpalettecolor (8,63,52,32); /* Light Brown */
#endif /* 0 */
}


/****************************************************************************
 * Check for "events"
 * If block, then busy-loop waiting for event, else check once and exit
 ***************************************************************************/
static errr CheckEvents(int block)
{
	int k = 0;

	if (block)
	{
		k = vga_getkey();
		if (k < 1) return (1);
	}
	else
		k = vga_getch();

	Term_keypress(k);

	return (0);
}


/****************************************************************************
 * Low-level graphics routine (assumes valid input)
 * Do a "special thing"
 ***************************************************************************/
static errr term_xtra_svgalib(int n, int v)
{
	switch (n)
	{
	case TERM_XTRA_EVENT:
		{
			/* Process some pending events */
			if (v) return (CheckEvents(FALSE));
			while (!CheckEvents(TRUE));
			return 0;
		}
	case TERM_XTRA_FLUSH:
		{
			/* Flush all pending events */
			/* Should discard all key presses but unimplemented */
			return 0;
		}
	case TERM_XTRA_CLEAR:
		{
			/* Clear the entire window */
			gl_fillbox(0, 0, 80 * CHAR_W, 25 * CHAR_H, 0);
			return 0;
		}
	case TERM_XTRA_DELAY:
		{
			/* Delay for some milliseconds */
			usleep(1000 * v);
			return 0;
		}
	}

	return 1;
}


/****************************************************************************
 * Low-level graphics routine (assumes valid input)
 * Draws a "cursor" at (x,y)
 ***************************************************************************/
static errr term_curs_svgalib(int x, int y)
{
	gl_fillbox(x * CHAR_W, y * CHAR_H, CHAR_W, CHAR_H, 15);

	return (0);
}


/****************************************************************************
 * Low-level graphics routine (assumes valid input)
 * Erases a rectangular block of characters from (x,y) to (x+w,y+h)
 ***************************************************************************/
static errr term_wipe_svgalib(int x,int y,int n)
{
	gl_fillbox(x * CHAR_W, y * CHAR_H, n * CHAR_W, CHAR_H, 0);
	return (0);
}


/****************************************************************************
 * Low-level graphics routine (assumes valid input)
 * Draw n chars at location (x,y) with value s and attribute a
 ***************************************************************************/
errr term_text_svgalib(int x, int y, int n, unsigned char a, cptr s)
{
	term_wipe_svgalib(x, y, n);
	gl_colorfont(8, 12, COLOR_OFFSET + (a & 0x0F)/*pal_trans[a]*/, font);
	gl_writen(x * CHAR_W, y * CHAR_H, n, (char *)s);

	return (0);
}


/****************************************************************************
 * Low-level graphics routine (assumes valid input)
 * Draw n chars at location (x,y) with value s and attribute a
 ***************************************************************************/
#ifdef USE_TRANSPARENCY
errr term_pict_svgalib(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp)
#else /* USE_TRANSPARENCY */
errr term_pict_svgalib(int x, int y, int n, const byte *ap, const char *cp)
#endif /* USE_TRANSPARENCY */
{
	int i;
	int x2, y2;

	for (i = 0; i < n; i++)
	{
		x2 = (cp[i] & 0x1F) * CHAR_W;
		y2 = (ap[i] & 0x1F) * CHAR_H;
		gl_copyboxfromcontext(buffer, x2, y2, CHAR_W,
			CHAR_H, (x + i) * CHAR_W, y * CHAR_H);
	}

	return (0);
}


void term_load_bitmap(void)
{
	void *temp;

	temp = read_bmp_file();

	/* Blit bitmap into buffer */
	gl_putbox(0, 0, bw, bh, temp);

	free(temp);

	return;
}


/****************************************************************************
 * Term hook
 * Initialize a new term
 ***************************************************************************/
static void term_init_svgalib(term *t)
{
	int VGAMODE, VIRTUAL;

	/* Hardwire this mode in for now */
	VGAMODE = G1024x768x256;
	VIRTUAL = 1;

	vga_init();

	/* Set up the bitmap buffer context */
	gl_setcontextvgavirtual(VGAMODE);
	buffer = gl_allocatecontext();
	gl_getcontext(buffer);

	/* Load bitmap into virtual screen */
	term_load_bitmap();

	/* Hardwire this mode in for now */
	VGAMODE = G640x480x256;

	/* Set up the physical screen context */
	if (vga_setmode(VGAMODE) < 0)
	{
		printf("Mode not available\n");
		exit(0);
	}

	gl_setcontextvga(VGAMODE);
	screen = gl_allocatecontext();
	gl_getcontext(screen);
	gl_enablepageflipping(screen);

	/* Set up palette colors */
	setpal();

	/* Load the character font data */
	initfont();

	/* Color 0 isn't transparent */
	gl_setwritemode(WRITEMODE_OVERWRITE);
}


/****************************************************************************
 * Term hook
 * Nuke an old term
 ***************************************************************************/
static void term_nuke_svgalib(term *t)
{
	vga_setmode(TEXT);
}


const char help_lsl[] = "SVGA (low level graphics) library";


/****************************************************************************
 * Hook SVGAlib routines into term.c
 ***************************************************************************/
errr init_lsl(int argc, char **argv)
{
	char path[1024];
	term *t = &term_screen_body;

	/* Unused parameters */
	(void)argc;
	(void)argv;

	if (arg_graphics)
	{
		use_graphics = TRUE;
	}

	/* Build the "graf" path */
	path_build(path, 1024, ANGBAND_DIR_XTRA, "graf");

	/* Allocate the path */
	ANGBAND_DIR_XTRA_GRAF = string_make(path);

	/* Initialize the term */
	term_init(t, 80, 24, 1024);

	/* The cursor is done via software and needs erasing */
	t->soft_cursor = TRUE;

	t->attr_blank = TERM_DARK;
	t->char_blank = ' ';

	/* Add hooks */
	t->init_hook = term_init_svgalib;
	t->nuke_hook = term_nuke_svgalib;
	t->text_hook = term_text_svgalib;

	if (use_graphics)
	{
		t->pict_hook = term_pict_svgalib;
		t->higher_pict = TRUE;
	}

	t->wipe_hook = term_wipe_svgalib;
	t->curs_hook = term_curs_svgalib;
	t->xtra_hook = term_xtra_svgalib;

	/* Save the term */
	term_screen = t;
	Term_activate(term_screen);

	return (0);
}

#endif /* USE_LSL */
