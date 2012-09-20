/* File: main-lsl.c */

/* Purpose: Support for Linux-SVGALIB Angband */

/*  Author: Jon Taylor (jon.taylor@gaia.ecs.csus.edu) */

/* Warning: XXX XXX XXX Not ready for Angband 2.7.9v2 */
/* See the TERM_XTRA_FLUSH and TERM_XTRA_CLEAR commands */

/***************************************************************************
 * Version: 1.3, 12/24/95
 * 
 * Some of the ideas in this code came from main-win.c by Skirmantas Kligys
 * (kligys@scf.usc.edu).
 ***************************************************************************/

/* Standard C header files */
#include <stdio.h>
#include <stdlib.h>

/* SVGAlib header files */
#include <vga.h>
#include <vgagl.h>
#include <vgakeyboard.h>

/* Angband header files */
#include "angband.h"

/* Define font/graphics cell width and height */
#define CHAR_W 8
#define CHAR_H 13

/* Define the number of columns in the source bitmap */
#define BM_COLS 64


/*** Begin "readbmp.h" ***/

/****************************************************************************
 * readbmp.h - BMP loader.
 *
 * Note -- this code needs to be stripped a bit... :-)
 *
 * Author: Jon Taylor (taylorj@gaia.ecs.csus.edu)
 * Based on original code by Carsten Engelmann (cengelm@gwdg.de)
 ***************************************************************************/

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

typedef struct BITMAPINFOHEADER{
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

typedef struct BITMAPCOREHEADER{
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

static int flip ();

void *read_bmp_file ()
{
  FILE *infile;
  int i, j;
  int iswindows = 0;
  int dummy, count, done, output_type;
  unsigned char *buf, *bmap, *pal;
  unsigned char read[2];
  unsigned char *p, *ptr, *dptr, *hptr;
  unsigned int w, h, palsize;
  char path[255];
  BITMAPINFOHEADER bih;
  BITMAPCOREHEADER bch;
  PICINFO *pp;
  pp=malloc(sizeof(PICINFO));
  
  bmap = NULL;
  pal = NULL;
  
  sprintf (path, "%s/8x13.bmp", ANGBAND_DIR_DATA);
  infile = fopen(path, "r");
  
  buf=(unsigned char *) malloc (54);

  fread (buf, 1, 26, infile);

  memcpy (&bch, buf + 14, 12);
  
  if (bch.bcSize == 40)		/* truly MS_Windows 3.x ?*/
    {
      fread (buf + 26, 1, 28, infile);/* then we need the rest */
      memcpy (&bih, buf + 14, 40);
      iswindows = TRUE;
    }
  else
    iswindows = FALSE;

  p=malloc (768);  
  pal = p;

  if (iswindows)		/*  MS_Windows 3.x */
    {
      pp->width = w = bih.biWidth;
      pp->height = h = bih.biHeight;
      pp->bpp = bih.biBitCount;

  /* Here the "offbits" -  we need 'em for the
     * palette size - e.g. XV uses different sizes
     */
      palsize = (*(unsigned *) (buf + 10) - 54) / 4;
    }
  else                         /*  OS/2 V1.1       */
    {
      pp->width = w = bch.bcWidth;
      pp->height = h = bch.bcHeight;
      pp->bpp = bch.bcBitCount;
      palsize = (*(unsigned *) (buf + 10) - 26) / 3;
    }

  if ((pp->bpp >> 3) < 3)
    output_type = 1;
  
  /* w is the size of a horizontal line in bytes
   * bih.biWidth is the number of valid pixels in a line
   * the latter one is passed to vgadisp.c as pp->width
   */
  switch (pp->bpp)
    {
    case 1:
      if (w % 32)
        w = (w / 32) * 32 + 32;;
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

  if ((pp->bpp == 24) && (output_type == 3))
    dummy = 3;
  else
    dummy = 1;
  bmap = malloc (w * (h + 2) * dummy);
  memset (bmap, 0, w * (h + 2) * dummy);

  switch (pp->bpp)
    {
    case 1:
      /* 1bit non compressed */
      ptr = pal;
          fread (ptr , 1, 3, infile);
          if (iswindows)
            fread (&dummy, 1, 1, infile);
          fread (ptr + 3, 1, 3, infile);
          if (iswindows)
            fread (&dummy, 1, 1, infile);
      for (i = 0; i < 2; i++)
        {
          p[i] = ptr[3 * i + 2];
          p[i + 256] = ptr[3 * i + 1];
          p[i + 512] = ptr[3 * i];
        }
      free (pal);
      pal = p;
      ptr = bmap;
	for (j = h - 1; j >= 0; j--)
	    for (i = 0, count=0 ; i < (w >> 3); i++)
             {
		hptr = ptr + j * pp->width;
                dummy = fgetc (infile);
                if (count < pp->width)
                  {
                    hptr[count] = (dummy & 128)?1:0;count++;
                    hptr[count] = (dummy & 64)?1:0;count++;
                    hptr[count] = (dummy & 32)?1:0;count++;
                    hptr[count] = (dummy & 16)?1:0;count++;
                    hptr[count] = (dummy & 8)?1:0;count++;
                    hptr[count] = (dummy & 4)?1:0;count++;
                    hptr[count] = (dummy & 2)?1:0;count++;
                    hptr[count] = dummy & 1;count++;
                  }
              }
        pp->numcols=2;
	break;
    case 4:
      /* 4bit non compressed */
      ptr = pal;
      for (i = 0; i < palsize; i++)
	{ 
	  fread (ptr + 3 * i, 1, 3, infile);
	  if (iswindows)
	    fread (&dummy, 1, 1, infile);
	}
      for (i = 0; i < palsize; i++)
	{
	  p[i] = ptr[3 * i + 2];
	  p[i + 256] = ptr[3 * i + 1];
	  p[i + 512] = ptr[3 * i];
	}
      free (pal);
      pal = p;
      ptr = bmap;
      if ((!iswindows) || (bih.biCompression == 0))
	{ 
	  for (j = h - 1; j >= 0; j--)
	    for (i = 0, count = 0; i < (w / 2); i++)
	      {
		dummy = fgetc (infile);
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
      else
	{
	  /* 4bit RLE compressed */
	  done = 0;
	  count = 0;
	  while (done == 0)
	    {
	      fread (read, 1, 2, infile);
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
		  ptr += fgetc (infile) + bih.biWidth * fgetc (infile);
		}
	      else
		{
		  dptr = hptr = (unsigned char *) malloc (read[1] >> 1);
		  fread (dptr, 1, read[1] >> 1, infile);
		  if (read[1] % 4 > 1)
		    dummy = fgetc (infile);
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
		  free (hptr);
		}
	    }
	  flip (*bmap, bih.biWidth, bih.biHeight);
	}
      pp->width = w;
      pp->numcols= 16;
      break;

    case 8:
      /* 8bit non compressed */
      ptr = pal;
      for (i = 0; i < palsize; i++)
	{
	  fread (ptr + 3 * i, 1, 3, infile);
	  if (iswindows)
	    dummy = fgetc (infile);
	}
      for (i = 0; i < palsize; i++)
	{
	  p[i] = ptr[3 * i + 2];
	  p[i + 256] = ptr[3 * i + 1];
	  p[i + 512] = ptr[3 * i];
	}
      free (pal);
      pal = p;
      ptr = bmap;
      if ((!iswindows) || (bih.biCompression == 0))
	for (i = h - 1; i >= 0; i--)
	  {
	    fread (ptr + pp->width * i, 1, w, infile);
	  }
      else
	/* 8bit RLE compressed */
	{
	  done = 0;
	  count = 0;
	  while (done == 0)
	    {
	      fread (read, 1, 2, infile);
	      if (read[0])
		for (i = 0; i < (int) read[0]; i++)
		  {
		    *ptr = read[1];
		    ptr++;
		  }
	      else if (read[1] == 0)
		{
		  count++;
		}
	      else if (read[1] == 1)
		done = 1;
	      else if (read[1] == 2)
		ptr += fgetc (infile) + bih.biWidth * fgetc (infile);
	      else
		{
		  fread (ptr, 1, read[1], infile);
		  if (read[1] % 2)
		    fgetc (infile);
		  ptr += read[1];
		}
	    }
	  flip (*bmap, bih.biWidth, bih.biHeight);
	}
      pp->numcols= 256;
      break;

    }

  free (buf);
/*  fclose (infile); */
  return (bmap);
}

static int flip (unsigned char * image, unsigned int w, unsigned int h)
{
  unsigned int i;
  unsigned char *hptr;

  hptr = (unsigned char *) malloc (w);
  for (i = 0; i < (h / 2); i++)
    {
      memcpy (hptr, image + i * w, w);
      memcpy (image + i * w, image + (h - i - 1) * w, w);
      memcpy (image + (h - i - 1) * w, hptr, w);
    }
  free (hptr);
  return (0);
}
/*** End "readbmp.h" ***/


/* ======================================================================= */

/* The main "term" structure */
static term term_screen_body;

/* The visible and virtual screens */
GraphicsContext *screen;
GraphicsContext *buffer;

/* The bitmap data */
void *bmap;

/* The font data */
void *font;

/* Initialize the screen font */
void initfont () {
  FILE *fontfile;
  void *temp;
  char path[255];
  long junk;

  /* Use an 8x12 font.  The graphic symbols are 8x13, so much hackage is
   * necessary to avoid problems from this.  Anyone have a bitmapped 8x13
   * font file laying around that they could send me? |-> */
  sprintf (path, "%s/8x12alt.psf", ANGBAND_DIR_DATA);
  if (!(fontfile = fopen(path,"r"))) {
    printf ("Error: could not open font file.  Aborting....\n");
    exit(1);
  }

  /* Junk the 4-byte header */
  fread (&junk,4,1,fontfile);
  
  /* Initialize font */
  temp = malloc (12*256);
  fread (temp,1,3328,fontfile);
  font = malloc (256*8*12*BYTESPERPIXEL);
  gl_expandfont (8,12,15,temp,font);
  gl_setfont (8,12,font);
  free (temp);
  
}


/* Initialize palette values for colors 0-15 */
void setpal() {
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
}

/* Translates raw palette index to Angband color index */
const char pal_trans[16]={0,3,13,11,2,7,12,5,1,10,15,4,14,6,9,8};


/****************************************************************************
 * Check for "events"
 * If block, then busy-loop waiting for event, else check once and exit
 ***************************************************************************/
static errr CheckEvents (int block) {
  int k=0;

  if (block) {
    k=vga_getkey();
    if (k<1) return (1);
  } else while ((k=vga_getkey())<1);
    
  Term_keypress(k);
  
  return(0);
}


/****************************************************************************
 * Low-level graphics routine (assumes valid input)
 * Do a "special thing"
 ***************************************************************************/
static errr term_xtra_svgalib (int n, int v) {
  
  switch (n) {
   case TERM_XTRA_EVENT: {
     if (v) return (CheckEvents (FALSE));
     while (!CheckEvents (TRUE));
     return (0);
   }
  }
  return(1);
  
}

/****************************************************************************
 * Low-level graphics routine (assumes valid input)
 * Draws a "cursor" at (x,y)
 ***************************************************************************/
static errr term_curs_svgalib (int x, int y, int z) {
  gl_fillbox (x*CHAR_W,y*CHAR_H,CHAR_W,CHAR_H-1,15); 
  return(0);
}

/****************************************************************************
 * Low-level graphics routine (assumes valid input) 
 * Erases a rectangular block of characters from (x,y) to (x+w,y+h)
 ***************************************************************************/
static errr term_wipe_svgalib (int x,int y,int w,int h) {
  gl_fillbox (x*CHAR_W,y*CHAR_H,w*CHAR_W,h*CHAR_H,0);
  return(0);
}

/* Translates "normal" attribute code to a bitmap offset */
unsigned int char_to_grfx (unsigned char a, unsigned char c) {
  if (c==32) return 32; /* Spaces don't have attributes */
  else return (c+(a-128)*128);
}

/****************************************************************************
 * Low-level graphics routine (assumes valid input)
 * Draw n chars at location (x,y) with value s and attribute a
 ***************************************************************************/
errr term_text_svgalib (int x, int y, int n, unsigned char a, cptr s) {
  unsigned int g;
  int i;
  
  /* If we see a "text" (a<=127) attribute, output colored text */
  if (a<=127) {  
    gl_colorfont (8,12,pal_trans[a],font);
    gl_writen (x*CHAR_W,y*CHAR_H,n,(char *)s);
    
  }
  
  /* Otherwise, we are displaying graphics */
  else {
    for (i=0;i<n;i++) {
      
      g=char_to_grfx (a,s[i]); /* Translates attribute into bitmap offset */
      
      /* Hack - draw only the top 12 lines of the graphic so that lines
       * aren't left lying around on the screen when character spaces from 
       * the 12-point font are used to overwrite 13-point graphic blocks. */
      gl_copyboxfromcontext (buffer,g%BM_COLS*CHAR_W,g/BM_COLS*CHAR_H,CHAR_W,
			     CHAR_H-1,(x+i)*CHAR_W,y*CHAR_H);
    }
  }
  
  return(0);
}

void term_load_bitmap() {
  void *temp;
  
  temp=read_bmp_file();
  gl_putbox(0,0,512,221,temp); /* Blit bitmap into buffer */
  free (temp);
  
  return;
  
}


/****************************************************************************
 * Term hook
 * Initialize a new term
 ***************************************************************************/
static void term_init_svgalib (term *t) {
  int VGAMODE, VIRTUAL;

  /* Initialize the low-level library.  As of now, vga_init() will fail due to
   * lack of appropriate I/O permissions unless angband is run as root, even 
   * though SVGALIB programs can be run by any user if they are suid root.  I 
   * think that this is caused by all the jacking around with setting and 
   * dropping privs in main.c, but I haven't been able to pin down exactly 
   * where the problem is as of yet.... */
  vga_init();
  VGAMODE=G640x480x256; /* Hardwire this mode in for now */
  VIRTUAL=1;
  
  /* Set up the bitmap buffer context */
  gl_setcontextvgavirtual(VGAMODE);
  buffer=gl_allocatecontext();
  gl_getcontext(buffer);
  
  term_load_bitmap(); /* Load bitmap into virtual screen */

  /* Set up the physical screen context */
  vga_setmode(VGAMODE);
  gl_setcontextvga(VGAMODE);
  screen=gl_allocatecontext();
  gl_getcontext(screen);

  initfont(); /* Load the character font data */

  gl_setwritemode(WRITEMODE_OVERWRITE);  /* Color 0 isn't transparent */

  setpal(); /* Set up palette colors */
}


/****************************************************************************
 * Term hook
 * Nuke an old term
 ***************************************************************************/
static void term_nuke_svgalib (term *t) {
  vga_setmode (TEXT); 
}


/****************************************************************************
 * Hook SVGAlib routines into term.c
 ***************************************************************************/
errr init_lsl (void) {
  term *t=&term_screen_body;
  
  term_init(t,80,24,1024); /* Initialize the term */
 
  use_graphics=TRUE; /* Show the wear/wield inventory as graphics */
  
  t->soft_cursor=TRUE; /* The cursor is done via software and needs erasing */
  
  /* Add hooks */
  t->init_hook=term_init_svgalib;
  t->nuke_hook=term_nuke_svgalib;
  t->text_hook=term_text_svgalib;
  t->wipe_hook=term_wipe_svgalib;
  t->curs_hook=term_curs_svgalib;
  t->xtra_hook=term_xtra_svgalib;
  
  term_screen=t; /* Save the term */
  Term_activate(term_screen);
  
  return(0);
}

