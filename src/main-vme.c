/* File: main-vme.c */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

/* Purpose: Support for "Vax Angband" */

/*
This is MAIN-VME C for VM/ESA machines.
First enable definition of VM in file "h-config.h"
You need to unpack archive EXT-VM VMARC .

To do that first set fixed file record length:
'COPYFILE EXT-VM VMARC A = = = ( REC F'
Then unpack:
VMARC UNPACK EXT-VM VMARC A

( if you don't have archivator vmarc, there is many places where you
get it, i got it from cc1.kuleuven.ac.be )

Then read MAKEFILE EXEC. It contains all neccessary information to
compile Angband.

EXT-VM VMARC content:
MAKEFILE EXEC
VMSERV TXTLIB
CNSHND ASSEMBLE

You will need about 3-4 MB free disk space to compile it.
If you have any problems, mail to

SM20616@vm.lanet.lv or SD30066@vm.lanet.lv

A large amount of this file appears to be a complete hack, but
what can you expect from a system designed for the Vax... :-)
 */


#include "angband.h"

#if defined(USE_VME) || defined(VM)


/*
 * Convert EBCDIC to ASCII
 */
char e2a[]=
{
	0x00, 0x01, 0x02, 0x03, 0x1A, 0x09, 0x1A, 0x7F, 0x1A, 0x1A, 0x1A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x1A, 0x0A, 0x08, 0x1A, 0x18, 0x19, 0x1A, 0x1A, 0x1C, 0x1D, 0x1E, 0x1F,
	0x1A, 0x1A, 0x1C, 0x1A, 0x1A, 0x0A, 0x17, 0x1B, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x05, 0x06, 0x07,
	0x1A, 0x1A, 0x16, 0x1A, 0x1A, 0x1E, 0x1A, 0x04, 0x1A, 0x1A, 0x1A, 0x1A, 0x14, 0x15, 0x1A, 0x1A,
	0x20, 0xA6, 0xE1, 0x80, 0xEB, 0x90, 0x9F, 0xE2, 0xAB, 0x8B, 0x9B, 0x2E, 0x3C, 0x28, 0x2B, 0x7C,
	0x26, 0xA9, 0xAA, 0x9C, 0xDB, 0xA5, 0x99, 0xE3, 0xA8, 0x9E, 0x21, 0x24, 0x2A, 0x29, 0x3B, 0x5E,
	0x2D, 0x2F, 0xDF, 0xDC, 0x9A, 0xDD, 0xDE, 0x98, 0x9D, 0xAC, 0xBA, 0x2C, 0x25, 0x5F, 0x3E, 0x3F,
	0xD7, 0x88, 0x94, 0xB0, 0xB1, 0xB2, 0xFC, 0xD6, 0xFB, 0x60, 0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,
	0xF8, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x96, 0xA4, 0xF3, 0xAF, 0xAE, 0xC5,
	0x8C, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x97, 0x87, 0xCE, 0x93, 0xF1, 0xFE,
	0xC8, 0x7E, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0xEF, 0xC0, 0xDA, 0x5B, 0xF2, 0xF9,
	0xB5, 0xB6, 0xFD, 0xB7, 0xB8, 0xB9, 0xE6, 0xBB, 0xBC, 0xBD, 0x8D, 0xD9, 0xBF, 0x5D, 0xD8, 0xC4,
	0x7B, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0xCB, 0xCA, 0xBE, 0xE8, 0xEC, 0xED,
	0x7D, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0xA1, 0xAD, 0xF5, 0xF4, 0xA3, 0x8F,
	0x5C, 0xE7, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0xA0, 0x85, 0x8E, 0xE9, 0xE4, 0xD1,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0xB3, 0xF7, 0xF0, 0xFA, 0xA7, 0xFF
};


/*
 * Convert ASCII to EBCDIC
 */
char a2e[]=
{
	0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F, 0x16, 0x05, 0x25, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26, 0x18, 0x19, 0x3F, 0x27, 0x22, 0x1D, 0x35, 0x1F,
	0x40, 0x5A, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D, 0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
	0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
	0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xAD, 0xE0, 0xBD, 0x5F, 0x6D,
	0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
	0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xC0, 0x4F, 0xD0, 0xA1, 0x07,
	0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F, 0x16, 0x05, 0x25, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26, 0x18, 0x19, 0x3F, 0x27, 0x22, 0x1D, 0x35, 0x1F,
	0x40, 0x5A, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D, 0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,
	0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
	0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xAD, 0xE0, 0xBD, 0x5F, 0x6D,
	0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
	0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xC0, 0x4F, 0xD0, 0xA1, 0x07
};


/*
 * System headers (already included by "angband.h" XXX XXX XXX)
 */
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* Some prototypes and definitions */

#define FIELD_SY 1
#define FIELD_SX 80
#define FIELD_EY 2
#define FIELD_EX 10

void GetAddr(int, int, char *);
void InitConsole(void);
void TerminateConsole(void);
void ResetScrBuf(void);
void AddScrBuf(char *, int);
char InKey(void);
char InKeyBuf(void);
void ScreenClear(void);
void ResetDISP(void);
int kbhit(void);
void ShowLine(int y, int x, int len);
void LoadProfile(void);

/* Changed lines marker. */

char DISP[26];

/* Max rows & columns number in screen (note that in older version
 ** than 2.7.9 we needed 25 lines, though 24th was unused. So it is set
 ** to 25 although screen actually has only 24 lines. (we just used to
 ** transfer 25th line into 24th.
 ** But in new version we no longer use it.
 */

int rows, cols;

/* Game cursor position on screen */

int curx=1;
int cury=1;

/*
 * Virtual Screen
 */
byte VirtualScreen[2048];
byte ScreenAttr[2048];

/*
 * This array is used for "wiping" the screen, initialized in "init_wat()"
 */
byte wiper[256];

/*
 * The main screen
 */
static term term_screen_body;


/* Update line on screen.
 ** Actually just set flag that we should update it.
 */
void ScreenUpdateLine(int line)
{
	DISP[line+1]=1;
}

/*
 * Clear the whole screen
 */
void ScreenClear(void)
{
	int iy;

	for (iy = 0; iy < rows; iy++)
	{
		memcpy(VirtualScreen + (iy*cols), wiper, cols);
		memset(ScreenAttr + (iy*cols), 0xF1, cols);
		ScreenUpdateLine(iy);
	}
}


/*
 * Nuke a Term
 */
static void Term_nuke_vm(term *t)
{
	TerminateConsole();
	puts("Console has been terminated(nuke).");
}

/*
 * Move the cursor
 */
static errr Term_curs_vm(int x, int y)
{
	/* Hack: mark line cursor was at as changed to ensure that old
	 ** cursor would be removed.
	 */
	DISP[cury]=1;
	curx=x+1;
	cury=y+1;
	return (0);
}


/*
 * The Angband color table
 *
 * Comment: White has been changed to blue to make screen look better.
 *          Green and Light Green are exchanged for the same reason.
 *          -0x80 means changing to bold font.
 *
 * Color Table:
 * Dark to Blue, White to Blue, Slate to Blue, Orange to Pink,
 * Red to Red, Green to GreenB, Blue to BlueB, Umber to Yellow,
 * Gray to Blue, Light White to WhiteB, Violet to PinkB, Yellow to Yellow
 * Light colors: Red to RedB,Green to Green,Blue to Cyan, Umber to YellB
 */
static const byte vm_color[] =
{

	0xF1,      0xF1,      0xF1,       0xF3,
	0xF2,      0xF4-0x80, 0xF1-0x80,  0xF6,
	0xF1,      0xF7-0x80, 0xF3-0x80,  0xF6,
	0xF2-0x80, 0xF4,      0xF5,       0xF6-0x80
};

/*
 * Place some text on the screen using an attribute
 */
static errr Term_text_vm(int x, int y, int n, byte a, cptr s)
{
	register int i;
	register byte attr;
	register byte *dest;
	register byte *dest2;

	/* Attribute... */
	attr = vm_color[a&0x0F];

	/* Paranoia */
	if (n > cols - x) n = cols - x;

	/* Access the destination */
	dest = VirtualScreen + ((cols * y) + x);
	dest2 = ScreenAttr + ((cols * y) + x);

	/* Virtually write the string */
	for (i = 0; (i < n) && s[i]; i++)
	{
		*dest++ = s[i];
		*dest2++ = attr;
	}

	/* Dump the line */
	ScreenUpdateLine(y);

	/* Success */
	return (0);
}


/*
 * Erase part of line.
 */
static errr Term_wipe_vm(int x, int y, int w)
{

	/* Paranoia -- Verify the dimensions */
	if (cols < (w+x)) w = (cols-x);

	/* Wipe part of the virtual screen, and update */
	memcpy(VirtualScreen + (y*cols + x), wiper, w);
	memset(ScreenAttr + (y*cols + x), 0xF1, w);
	ScreenUpdateLine(y);

	/* Success */
	return (0);
}


/*
 * Handle special request.
 */
static errr Term_xtra_vm(int n, int v)
{
	int i, j;
	char tmp;

	/* Analyze the request */
	switch (n)
	{

		/* Make a noise */
		case TERM_XTRA_NOISE:

		/* No noises here! :) */
		return (0);

		/* Wait for a single event */
		case TERM_XTRA_EVENT:

		/* No wait key press check */
		if (!v && !kbhit()) return (1);

		/* Wait for a keypress */
		i = getkey();

		/* Save keypress */
		Term_keypress(i);

		/* Success */
		return (0);

		case TERM_XTRA_CLEAR:

		ScreenClear();
		return (0);

#if 0
		case TERM_XTRA_FROSH:
		ScreenUpdateLine(VirtualScreen + (cols*v), v);
		return (0);
#endif

		case TERM_XTRA_FLUSH:

		/* Flush keys */
		while (1)
		{
			tmp=getkeybuf();
			if (!tmp) break;
			Term_keypress(tmp);
		}

		/* Success */
		return (0);
	}

	/* Unknown request */
	return (1);
}


/*
 * Initialize the VM/CNSconsole.
 */
errr init_vme(void)
{
	register i;

	term *t = &term_screen_body;

	short blank = ' ';

	static int done = FALSE;

	/* Paranoia -- Already done */
	if (done) return (-1);

	/* Build a "wiper line" of blank spaces */
	for (i = 0; i < 256; i++) wiper[i] = blank;

	/* Acquire the size of the screen */
	rows = 25;
	cols = 80;

	/* Initialize the console */
	InitConsole();

	/* Wipe the screen */
	for (i = 0; i < rows; i++)
	{
		/* Wipe that row */
		memcpy(VirtualScreen + (i*cols), wiper, cols);
		memset(ScreenAttr + (i*cols), 0xF1, cols);
	}

	/* Erase the screen */
	ScreenClear();

	/* Initialize the term -- very large key buffer */
	term_init(t, cols, rows - 1, 1024);

	/* Prepare the init/nuke hooks */
	t->nuke_hook = Term_nuke_vm;

	/* Connect the hooks */
	t->text_hook = Term_text_vm;
	t->wipe_hook = Term_wipe_vm;
	t->curs_hook = Term_curs_vm;
	t->xtra_hook = Term_xtra_vm;

	/* Save it */
	term_screen = t;

	/* Activate it */
	Term_activate(term_screen);

	/* Done */
	done = TRUE;

	/* Success */
	return 0;
}

/* Wait for keypress */
int
getkey(void)
{
	return ((int)InKey());
}

/* Read key buffer if not empty */

int
getkeybuf(void)
{
	return ((int)InKeyBuf());
}


/*********************************************************************/
/*********************************************************************/
/******************   Actual work with console   *********************/
/*********************************************************************/


/* Low-level functions */
int  CNSINIT(char *path, int device);
int  CNSTERM(char *path);
int  CNSREAD(char *path, char *buffer, int buflen);
int  CNSWRITE(char *path, char *buffer, int buflen);

#define _PF1    (0xF1)
#define _PF2    (0xF2)
#define _PF3    (0xF3)
#define _PF4    (0xF4)
#define _PF5    (0xF5)
#define _PF6    (0xF6)
#define _PF7    (0xF7)
#define _PF8    (0xF8)
#define _PF9    (0xF9)
#define _PF10   (0x7A)
#define _PF11   (0x7B)
#define _PF12   (0x7C)
#define _PF13   (0xC1)
#define _PF14   (0xC2)
#define _PF15   (0xC3)
#define _PF16   (0xC4)
#define _PF17   (0xC5)
#define _PF18   (0xC6)
#define _PF19   (0xC7)
#define _PF20   (0xC8)
#define _PF21   (0xC9)
#define _PF22   (0x4A)
#define _PF23   (0x4B)
#define _PF24   (0x4C)

#define _PA2    (0x6E)
#define _PA3    (0x6B)

#define _CLEAR  (0x60)

#define _ENTER  (0x7D)

extern char cnscrstb[];   /* Hardware 3270 cursor offsets */

/* Console identificator */
char *cons="console";

/* Console interrupt flag; should be set by assembler patch */
int CNSINTR;
/* Assembler function prototype; this handles console interrupt */
extern int CNSHD();

/* Buffer for output stream (VM/ESA uses streams to control terminals */
static char * ScrBuf;
/* Incoming buffer from console after cnsread */
static char * InBuf;
/* User entered command buffer (paranoia: make it so long no one will
 ** ever run short of it :)
 */
static char ComBuf[256];
/* Pointer to current position in ComBuf */
static char * ComPtr;
/* This array is used to clean up input field.
 ** Comment: erase everything user entered after we accepted it.
 */
static char wiping[]=
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
/* Flag: should we wipe input field ? */
static int wipe=0;
/* Counter: how many times did game checked if we pressed any key.
 ** Comment: after every 100th we update the screen.
 ** Comment2: because of slow work with screen it would be _very_
 **           unwise to make it more often.
 */
static int kbhitcount;
/* Keep length of collected output stream */
static int BufLen;
/* Define array which will be used to bring actual cursor
 ** (not game cursor!!!) back to the beginning of input field.
 */
static char CursorHome[]=
{0, 0, 0, 0x13};
/* Define array to make fields on the screen.
 ** Comment: 2 fields: input field(writable), and game field(protected).
 ** Comment2: We will change PSS settings into dumb repeating of color
 **           setting if PSS fonts are not abailable.
 */
static char ScrField[]=
{0, 0, 0, 0x29, 0x03, 0xC0, 0xC1, 0x42, 0xF3, 0x43, 0xC1, \
0, 0, 0, 0x29, 0x03, 0xC0, 0x60, 0x42, 0xF1, 0x43, 0xC1};
/* Last command user entered (for repeating purposes */
static char LastCmd[256];
/* Array which holds PFkeys definitions */
static char PFcmd[25][80];


/* Actually initialize console ... */
void InitConsole(void)
{
	/* Use this to activate full screen mode after console has been
	 ** initialized.
	 */
	unsigned char init[6] =
	{0xC2, 0x11, 0x40, 0x40, 0, 0};
	int i;
	/* Array for checking is PSS fonts are loaded */
	char pss[256];

	/* Allocate memory */
	ScrBuf=malloc(4100);
	if (ScrBuf==NULL)
	{
		puts("Cannot mallocate memory for screen buffer!");
		exit(77);
	}
	/* Block system standarts... */
	system("cp set msg off");
	system("cp set emsg off");
	system("cp set imsg off");
	system("cp term brkkey none");
	/* Test PSS */
	system("desbuf");
	system("query display (stack");
	gets(pss);
	i=1;
	if (pss[63]!='P') i=0;
	if (pss[64]!='S') i=0;
	if (pss[65]!='S') i=0;
	if (pss[72]!='0') i=0;
	if (pss[73]!='1') i=0;
	if (pss[74]!='A') i=0;
	if (pss[75]!='B') i=0;
	if (i==0)
	{
		/* No PSS. Cannot run without them... */
		puts("ERROR: Cannot run without PSS fonts!");
		exit(77);
	}
	ScrBuf[0]=0xC2;
	ScrBuf[1]=0;
	BufLen=1;
	/* Allocate memory */
	InBuf=malloc(200);
	if (InBuf==NULL)
	{
		puts("Cannot mallocate memory for screen buffer!");
		exit(77);
	}
	/* Okay, lets make some intial assigments */
	InBuf[0]=0;
	ComBuf[0]=0;
	ComPtr=ComBuf;
	LastCmd[0]=0;
	GetAddr(FIELD_SY, FIELD_SX, ScrField);
	GetAddr(FIELD_EY, FIELD_EX, ScrField+11);
	GetAddr(FIELD_SY, FIELD_SX+1, wiping);
	GetAddr(FIELD_SY, FIELD_SX+1, CursorHome);
	/* Initialize console */
	cnsxinit(cons, 0x9, 1, CNSHD);
	/* Activate full-screen mode */
	cnswrite(cons, init, 6);
	/* No lines on screen were changed */
	for (i=1; i<25; i++)
		DISP[i]=0;
	/* Corsor home... */
	AddScrBuf(CursorHome, sizeof(CursorHome));
	/* No console interrupt yet */
	CNSINTR=0;
	/* No check keypresses yet */
	kbhitcount=0;
	/* Let's load 'profile angband' for PFs settings */
	LoadProfile();
}

void TerminateConsole(void)
{
	free(ScrBuf);
	free(InBuf);
	/* Terminate console */
	cnsterm(cons);
	/* Restore system defaults.
	 ** Comment: better we should have written them down at start but
	 ** it is reasonably difficult to make, so just set standart values.
	 */
	system("cp set msg on");
	system("cp set emsg on");
	system("cp set imsg on");
	system("cp term brkkey pa1");
}

/* Reset collected output stream */
void ResetScrBuf(void)
{
	int i;
	unsigned char attr;

	/* Where shall cursor be ? */
	GetAddr(cury, curx, ScrBuf+BufLen);
	BufLen+=3;
	/* Actually 'make' cursor : underline symbol */
	ScrBuf[BufLen++]=0x28;
	ScrBuf[BufLen++]=0x41;
	ScrBuf[BufLen++]=0xF4;
	attr=*(ScreenAttr+(cury-1)*cols+curx-1);
	ScrBuf[BufLen++]=0x28;
	ScrBuf[BufLen++]=0x42;
	ScrBuf[BufLen++]=attr|0x80;
	ScrBuf[BufLen++]=0x28;
	ScrBuf[BufLen++]=0x43;
	if (attr>0x80) ScrBuf[BufLen++]=0xC1;
	else ScrBuf[BufLen++]=0xC2;
	ScrBuf[BufLen++]=*(VirtualScreen+(cury-1)*cols+curx-1);
	ScrBuf[BufLen++]=0x28;
	ScrBuf[BufLen++]=0x41;
	ScrBuf[BufLen++]=0x00;
	ScrBuf[BufLen++]=0x28;
	ScrBuf[BufLen++]=0x43;
	ScrBuf[BufLen++]=0xC1;
	/* Draw screen fields.
	 ** Comment: paranoia, should always be there, but won't be any worse
	 **          if we redraw them.
	 */
	memcpy(ScrBuf+BufLen, ScrField, 22);
	if (wipe)
	{
		/* If we should wipe input field, let's do it. */
		wipe=0;
		memcpy(ScrBuf+BufLen+22, wiping, sizeof(wiping));
		/* Actually write to terminal */
		cnswrite(cons, ScrBuf, BufLen+22+sizeof(wiping));
	}
	else
	{
		/* Actually write to terminal */
		cnswrite(cons, ScrBuf, BufLen+22);
	}
	/* Discard old output stream */
	ScrBuf[0]=0xC2;
	ScrBuf[1]=0;
	BufLen=1;
}

/* Just add some more to output stream */
void AddScrBuf(char * ptr, int len)
{
	/* Stream cannot be longer than 4k, so reset it */
	if (len+BufLen>4000) ResetScrBuf();
	memcpy(ScrBuf+BufLen, ptr, len);
	BufLen+=len;
}

/* Calculate codes for cursor setting and write them down into *stream.
 ** Comment: system has _really crazy tables for this calculation so
 **          we need function for this.
 */
void GetAddr(int y, int x, char *stream)
{
	int sx, sy, len;

	len = y * 80 + x - 81;

	sx = len & 0x3F;
	sy = len >> 6;

	*stream++ = 0x11;
	/* Use standart array for cursor offsets (VMSERV TXTLIB) */
	*stream++ = cnscrstb[sy];
	*stream = cnscrstb[sx];
}

/* Okay, program requested pressed key, let's return it */
char InKey(void)
{
	char ret;
	char * info;
	int i;
	char *ptr, *ptrs;
	int PF;
	int prev;

	/* If we didn't finish previous line user entered just return
	 ** next symbol.
	 ** Comment: unlikely IBM PC and others user may enter string
	 **          of any length (limited by input field length to 9
	 **          symbols) and only them by pressing ENTER key or
	 **          functional key make real console interrupt.
	 **          So we should handle everything user entered.
	 */
	if (*ComPtr)
	{
		ret=*ComPtr;
		ComPtr++;
		return (ret);
	}
	/* If we get here than user command buffer is empty and we have
	 ** actually wait for new command.
	 */
	/* Okay, update screen before :) */
	ResetDISP();
	/* Well, now wait till user enters something and read it. */
	cnsread(cons, InBuf, 200);
	/* Oh, we already handling interrupt so, reset interrupt flag */
	CNSINTR=0;
	/* No previous cmd inserted yet */
	prev=0;
	/* User pressed CLEAR key. Redraw all screen then. */
	if (*InBuf==0x6D)
	{
		/* Hack: mark all lines as changed */
		for (i=1; i<26; i++)
			DISP[i]=1;
		/* Cursor home */
		AddScrBuf(CursorHome, sizeof(CursorHome));
		/* Redraw the whole screen */
		ResetDISP();
		/* Hack: we should return something, shouldn't we?
		 ** Just return CR. It seems safest.
		 */
		return (13);
	}
	/* Okay parse stream from console to make string only of
	 ** user's input.
	 */
	info=memchr(InBuf, 0x1D, 200);
	info+=2;
	info[9]=0;
	/* Mark that we should clear input field */
	wipe=1;
	/* Cursor home */
	AddScrBuf(CursorHome, sizeof(CursorHome));
	/* Pointer to current letter in buffered command set to start */
	ComPtr=ComBuf;
	/* Clear length buffer */
	ComBuf[0]=0;
	switch (*InBuf)
	{
		/* PA2 & PA1 */
		case 0x6E:;
		case 0x6C:
		/* These 2 keys are supposed to mean 'ESC' */
		return (27);
		break;
		/* Hack: determine which PF key was exactly pressed.
		 */
		case _PF1:
		PF=1;
		break;
		case _PF2:
		PF=2;
		break;
		case _PF3:
		PF=3;
		break;
		case _PF4:
		PF=4;
		break;
		case _PF5:
		PF=5;
		break;
		case _PF6:
		PF=6;
		break;
		case _PF7:
		PF=7;
		break;
		case _PF8:
		PF=8;
		break;
		case _PF9:
		PF=9;
		break;
		case _PF10:
		PF=10;
		break;
		case _PF11:
		PF=11;
		break;
		case _PF12:
		PF=12;
		break;
		case _PF13:
		PF=13;
		break;
		case _PF14:
		PF=14;
		break;
		case _PF15:
		PF=15;
		break;
		case _PF16:
		PF=16;
		break;
		case _PF17:
		PF=17;
		break;
		case _PF18:
		PF=18;
		break;
		case _PF19:
		PF=19;
		break;
		case _PF20:
		PF=20;
		break;
		case _PF21:
		PF=21;
		break;
		case _PF22:
		PF=22;
		break;
		case _PF23:
		PF=23;
		break;
		case _PF24:
		PF=24;
		break;
		/* Uh, user pressed ENTER.
		 ** Determine if we should add 'CR' at the end or not ?
		 */
		case _ENTER:
		PF=0;
		/* Copy entered string into buffer */
		strcpy(ComBuf, info);
		/* Never end strings of length 1 or 0 with CR */
		if (strlen(ComBuf)<2) break;
		/* If string ends with R* or R& add CR */
		ptr=info+strlen(info)-1;
		if (*ptr=='*'||*ptr=='&')
		{
			ptr--;
			if (ptr<info) break;
			if (*ptr!='R') break;
			ptr=ComBuf+strlen(ComBuf);
			ptr[0]=13;
			ptr[1]=0;
			break;
		}
		/* Well, only numbers should be padded with CR.
		 ** Comment: handle 18/... too.
		 */
		if (!isdigit(*ptr)) break;
		ptr--;
		i=1;
		while (ptr>=info)
		{
			if (*ptr=='R') break;
			if (*ptr=='@'&&ptr==info)
				break;
			if (*ptr=='/')
			{
				i=0;
				if (*(ptr-1)!='8') break;
				if (*(ptr-2)!='1') break;
				if ((ptr-2)!=info) break;
				i=1;
				break;
			}
			if (!isdigit(*ptr))
			{
				i=0;
				break;
			}
			ptr--;
		}
		if (i)
			ptr=ComBuf+strlen(ComBuf);
		ptr[0]=13;
		ptr[1]=0;
		break;
		default:
		PF=0;
	}
	/* Okay let's proceed with parsing PFkeys commands */
	ptrs=PFcmd[PF];
	ptr=ComBuf;
	/* If key actually was PF and we didn't run into end of PF
	 ** command definition...
	 */
	while (*ptrs && PF)
	{
		/* If not '\' just copy symbol into buffer */
		if (*ptrs!='\\')
		{
			*ptr=*ptrs;
			ptr++;
			ptrs++;
			continue;
		}
		/* We have discovered '\'. Let's resolve it. */
		ptrs++;
		switch (*ptrs)
		{
			/* End of line ? Do nothing then. */
			case 0:
			break;
			/* User actually wants '\' */
			case '\\':
			*ptr='\\';
			ptr++;
			break;
			/* User wants his previous command inserted. */
			case 'p':
			/* Set flag that we used previous command */
			prev=1;
			strcpy(ptr, LastCmd);
			ptr=ComBuf+strlen(ComBuf);
			break;
			/* Insert string user actually entered */
			case 's':
			strcpy(ptr, info);
			ptr=ComBuf+strlen(ComBuf);
			break;
			/* User wants 'CR' */
			case 'n':
			*ptr=13;
			ptr++;
			break;
			/* Simulate CTRL key pressing */
			case '^':
			ptrs++;
			if (!(*ptrs)) break;
			*ptr=KTRL(*ptrs);
			ptr++;
			break;
			/* ESC */
			case 'c':
			*ptr=27;
			ptr++;
			break;
			/* Hmm, urecognized command. Just copy letter. */
			default:
			*ptr=*ptrs;
			ptr++;
		}
		if (ptrs) ptrs++;
	}
	/* Terminate string with 0 if neccessary */
	if (PF) *ptr=0;
	/* Empty string? User probably wants to send CR. */
	if (!(*ComBuf))
	{
		return (13);
	}
	/* Okay, set pointer to next letter */
	ComPtr++;
	/* Backup last command if not empty */
	ptr=ComBuf;
	/* String still "empty" */
	i=1;
	while ((*ptr)&&i)
	{
		if ((*ptr!=13)&&(*ptr!=27)&&(*ptr!=' '))
			i=0;
		ptr++;
	}
	/* Never update previous command if we inserted it.
	 ** Comment: this is to avoid recursively multiplying previous
	 **          commands.
	 */
	if ((!i)&&(!prev))
		strcpy(LastCmd, ComBuf);
	/* Return something */
	return (*ComBuf);
}

/* Get symbol from buffer if exists */

char InKeyBuf(void)
{
	char ret;

	ret=*ComPtr;
	if (ret) ComPtr++;
	return (ret);
}

/* Here we draw all changed lines */
void ResetDISP(void)
{
	int i;

	/* No need to redraw screen in near future */
	kbhitcount=0;
	for (i=1; i<25; i++)
		if (DISP[i]!=0)
	{
		/* If line was changed, it won't stay changed any more */
		DISP[i]=0;
		/* We don't want to erase input field definitons from
		 ** screen, so check it carefully.
		 */
		switch (i)
		{
			case FIELD_SY:
			ShowLine(i, 1, FIELD_SX-1);
			if (i==FIELD_EY)
			{
				ShowLine(i, FIELD_EX+1, 80-FIELD_EX);
			}
			break;
			case FIELD_EY:
			ShowLine(i, FIELD_EX+1, 80-FIELD_EX);
			break;
			default:
			ShowLine(i, 1, 80);
		}
	}
	/* Okay, actually show something */
	ResetScrBuf();
}

/* Did user press something? */
int kbhit(void)
{
	kbhitcount++;
	if (kbhitcount>99)
	{
		/* We waited long enough, time to redraw screen.
		 ** Comment: during sleep every 100th turn is shown.
		 */
		kbhitcount=0;
		ResetDISP();
	}
	return (CNSINTR);
}

/* Let's show requested line(or part of it). */
void ShowLine(int y, int x, int len)
{
	char slbuf[512];
	char *ptr, *scr, *attr;
	int i;
	unsigned char curattr;

	ptr=slbuf+3;
	/* Hack: was used for older version to place 25th string on
	 ** 24th.
	 */
	if (y<25)
		GetAddr(y, x, slbuf);
	else
		GetAddr(24, x, slbuf);
	/* Set some adresses */
	curattr=0;
	scr=VirtualScreen+(y-1)*cols+x-1;
	attr=ScreenAttr+(y-1)*cols+x-1;
	/* Let's proceed with string. */
	for (i=0; i<len; i++)
	{
		/* Attribute has changed ? Let's add codes for changing color */
		if (*attr!=curattr)
		{
			/* Pss has changed ? Let's change pss. */
			if (((*attr)&0x80)!=(curattr&0x80)||(!curattr))
			{
				*ptr++=0x28;
				*ptr++=0x43;
				if (*attr>0x80) *ptr++=0xC1;
				else *ptr++=0xC2;
			}
			/* Color has changed ? Let's change it. */
			if (((*attr)&0x7F)!=(curattr&0x7F))
			{
				*ptr++=0x28;
				*ptr++=0x42;
				if (*attr>0x80) *ptr++=*attr;
				else *ptr++=*attr+0x80;
			}
			curattr=*attr;
		}
		*ptr++=*scr;
		scr++;
		attr++;
	}
	/* Add string to output stream */
	AddScrBuf(slbuf, ptr-slbuf);
}

void LoadProfile(void)
{

	FILE *fp;
	char line[128], *ptr, *pfptr, *p;
	int pf, i;

	for (i = 1; i <= 24; i++) strcpy(PFcmd[i], "\\s");

	fp = fopen("PROFILE ANGBAND", "r");
	if (!fp) return;
	{   while (fgets(line, 128, fp))
		{   if (*line == '#') continue;
			ptr = strstr(line, "PF");
			if (!ptr) continue;
			ptr += 2;
			p = ptr;
			while (isdigit(*p)) ++p;
			*p++ = 0;
			pf = atoi(ptr);
			if (pf < 1 || pf > 24) continue;
			ptr = strchr(p, '"');
			++ptr;
			p = strchr(ptr, '"');
			if (!p)
			{   puts("'\"' missing in PROFILE ANGBAND.");
				continue;
			}
			*p = 0;
			strcpy(PFcmd[pf], ptr);
		}
	}
}

/*
 **
 ** File descriptor emulation for VM/ESA
 **
 **
 */

#include <string.h>
#include <stdio.h>
#include "fcntl.h"

static FILE *file_descriptors[40];

int
open(char *name, int flags, int mode)
{
	char fmode[16];
	FILE *fp;
	int i;

	strcpy(fmode, "rb");
	if ((flags & O_WRONLY) || (flags & O_RDWR)) strcpy(fmode, "wb+");

	fp = fopen(name, fmode);

	if (!fp) return (-1);
	for (i = 1; i < 40; i++)
		if (!file_descriptors[i])
	{   file_descriptors[i] = fp;
		return (i);
	}
	return (-1);
}

void
close(int fd)
{
	fclose(file_descriptors[fd]);
}

int
read(int fd, char *buff, int bytes)
{
	return (fread(buff, 1, bytes, file_descriptors[fd]));
}

int
write(int fd, char *buff, int bytes)
{
	return (fwrite(buff, 1, bytes, file_descriptors[fd]));
}

long
lseek(int fd, long pos, int set)
{
	return (fseek(file_descriptors[fd], pos, set));
}

void
unlink(char *filename)
{
	remove(filename);
}


#endif /* USE_VME */

