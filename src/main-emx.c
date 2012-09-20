/* File: main-emx.c */

/*
 * Copyright (c) 1997 Ben Harrison, Ekkehard Kraemer, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

/* Purpose: Support for OS/2 EMX Angband */

/* Author: ekraemer@pluto.camelot.de (Ekkehard Kraemer) */

/* Current maintainer: silasd@psyber.com (Silas Dunsmore) */
/* Unless somebody else wants it.... */

#ifdef USE_EMX

/*
 * === Instructions for using Angband 2.7.X with OS/2 ===
 *
 * The patches (file "main-emx.c") to compile Angband 2.7.X under OS/2
 * were written by ekraemer@pluto.camelot.de (Ekkehard Kraemer).
 *
 * TO COMPILE:
 *
 * - untar the archive into /angband (or /games/angband or whatever)
 * - change directory to /angband/src
 * - run "dmake -B -r -f makefile.emx" (not gmake or make)
 *
 * TO INSTALL:
 *
 * - change directory to /angband/src
 * - run "dmake -B -r -f makefile.emx install" (not gmake or make)
 * - copy your old savefile into ./lib/save and your old pref.prf into ./lib/user
 * - start /angband/angband.exe for one single window
 * - start /angband/startwnd.cmd for multiple windows
 *
 * TO REMOVE TEMPORARY FILES:
 *
 * - run 'dmake -B -r -f makefile.emx clean'
 *
 *
 * I use EMX 0.9b, but every EMX compiler since 0.8g or so should work
 * fine. EMX is available at ftp-os2.cdrom.com ("Hobbes"), as is dmake.
 *
 *  dmake:    ftp://ftp-os2.cdrom.com/all/program/dmake38X.zip
 *  EMX:      ftp://ftp-os2.cdrom.com/2_x/unix/emx???/   (most probably)
 *
 * Old savefiles must be renamed to follow the new "savefile" naming
 * conventions.  Either rename the savefile to "PLAYER", or start the
 * program with the "-uName" flag.  See "main.c" for details.  The
 * savefiles are stored in "./lib/save" if you forgot the old names...
 *
 *  Changes
 *  =======
 *
 *  When       By     Version   What
 *  -------------------------------------------------------------------
 *
 *  18.11.95   EK      2.7.8    Added window/pipe code
 *                              Introduced __EMX__CLIENT__ hack
 *
 *  15.12.95   EK      2.7.9    Updated for 2.7.9
 *                     beta     Added third view
 *                              Added number of line support in aclient
 *
 *  25.12.95   EK      2.7.9    Added 'install' target
 *                    non-beta  Updated installation hints
 *                              Uploaded binary to export.andrew.cmu.edu
 *
 *  25.01.96   EK      2.7.9    Updated for 2.7.9v3
 *                      v3      Removed (improved) keyboard hack
 *                              Introduced pref-emx.prf
 *                              Phew... Makefile.emx grows! (patches, export)
 *                              Uploaded binary to export.andrew.cmu.edu
 *
 *  26.01.96   EK               Added files.uue target
 *
 *  22.02.96   EK               Added PM support
 *
 *   2.03.96   EK      2.7.9    Uploaded binaries to export.andrew.cmu.edu
 *                      v4
 *
 *   9.03.96   EK      2.7.9    Adjustable background color (PM)
 *                      v5      Added map window
 *
 *  3 Dec 97   SWD      282     Brought key-handling, macros in sync with DOS.
 *                              Hacked on sub-window code -- it compiles, but
 *                              doesn't link.
 *
 * 23 Jan 98   SWD      282     Hacked more on sub-windows.  Now links, with
 *                              warnings.  Seems to work.
 *
 * 25 May 99   GDL      283     Moved pipe and PM related data into Term struct to avoid
 *					  term conflicts (redefintion, etc).  Some code cleanup.
 *
 * 02 Jun 99   GDL      283     Fixed handling of sub-windows so that they tell server
 *					  how big they are, avoiding sub-window display corruption.
 *
 */

#include "main.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/kbdscan.h>
#include <io.h>
#include <process.h>
#define INCL_KBD 1
#define INCL_WINWINDOWMGR
#define INCL_WINSWITCHLIST
#include <os2.h>
#include <sys/video.h>

#include "angband.h"


/*
 * Maximum windows
 */
#define MAX_TERM_DATA 8

/*
 * Keypress input modifier flags (copied from main-ibm.c)
 *
 * SWD: these could be changed to the definitions in <os2.h>, which are
 *      direct bitmasks instead of shift-counts.
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
 * Prototypes!
 */
static errr Term_curs_emx(int x, int y);
static errr Term_wipe_emx(int x, int y, int n);
static errr Term_text_emx(int x, int y, int n, unsigned char a, cptr s);
static void Term_init_emx(term *t);
static void Term_nuke_emx(term *t);

/*
 * the terms!  kind of useless for client.  Oh well.
 */

static term emxterm[MAX_TERM_DATA];

#ifndef EMXPM

/*
 * Use the "data" field in the term struct to keep track of the
 * pipe handle
 */

typedef struct
{
	FILE *out;                  /* used by the ..._pipe_emx stuff */
} termData;

/*
 * Communication between server and client
 */

enum
{
	PIP_INIT,
	PIP_NUKE,
	PIP_XTRA,
	PIP_CURS,
	PIP_WIPE,
	PIP_TEXT,
};


/*
 * Current cursor "size"
 */
static int curs_start=0;
static int curs_end=0;

/*
 * Angband color conversion table
 *
 * Note that "Light Green"/"Yellow"/"Red" are used for
 * "good"/"fair"/"awful" stats and conditions.
 *
 * But "brown"/"light brown"/"orange" are really only used
 * for "objects" (such as doors and spellbooks), and these
 * values can in fact be re-defined.
 *
 * Future versions of Angband will probably allow the
 * "use" of more that 16 colors, so "extra" entries can be
 * made at the end of this table in preparation.
 */
static int colors[16]=
{
	F_BLACK,                    /* Black */
	F_WHITE|INTENSITY,          /* White */
	F_WHITE,                    /* XXX Gray */
	F_RED|INTENSITY,            /* Orange */
	F_RED,                      /* Red */
	F_GREEN,                    /* Green */
	F_BLUE,                     /* Blue */
	F_BROWN,                    /* Brown */
	F_BLACK|INTENSITY,          /* Dark-grey */
	F_WHITE,                    /* XXX Light gray */
	F_MAGENTA,                  /* Purple */
	F_YELLOW|INTENSITY,         /* Yellow */
	F_RED|INTENSITY,            /* Light Red */
	F_GREEN|INTENSITY,          /* Light Green */
	F_BLUE|INTENSITY,           /* Light Blue */
	F_BROWN|INTENSITY           /* Light brown */
};

/*
 * Display a cursor, on top of a given attr/char
 */
static errr Term_curs_emx(int x, int y)
{
	v_gotoxy(x, y);
	v_ctype(curs_start, curs_end);

	return (0);
}

/*
 * Erase a grid of space (as if spaces were printed)
 */
static errr Term_wipe_emx(int x, int y, int n)
{
	v_gotoxy(x, y);
	v_putn(' ', n);

	return (0);
}

/*
 * Draw some text, wiping behind it first
 */
static errr Term_text_emx(int x, int y, int n, unsigned char a, cptr s)
{
	/* Convert the color and put the text */
	v_attrib(colors[a & 0x0F]);
	v_gotoxy(x, y);
	v_putm(s, n);

	return (0);
}

/*
 * EMX initialization
 */
static void Term_init_emx(term *t)
{
	struct _KBDINFO kbdinfo;	/* see structure description ?somewhere? */

	v_init();
	v_getctype(&curs_start, &curs_end);
	/* hide cursor (?) XXX XXX XXX */
	v_clear();

	/* the documentation I (SWD) have implies, in passing, that setting */
	/* "binary mode" on the keyboard device will prevent the O/S from */
	/* acting on keys such as ^S (pause) and ^P (printer echo). */

	/* note also that "KbdSetStatus is ignored for a Vio-windowed application." */
	/* so there may well be problems with running this in a window.  Damnit. */

	/* this is kind of a nasty structure, as you can't just flip a bit */
	/* to change binary/ASCII mode, or echo on/off mode... nor can you */
	/* clear the whole thing -- certain bits need to be preserved. */

	KbdGetStatus(&kbdinfo, (HKBD)0);
	kbdinfo.fsMask &= ~ (KEYBOARD_ECHO_ON|	/* clear lowest four bits */
		KEYBOARD_ECHO_OFF|KEYBOARD_BINARY_MODE|KEYBOARD_ASCII_MODE);
	kbdinfo.fsMask |= (KEYBOARD_BINARY_MODE);	/* set bit two */
	KbdSetStatus(&kbdinfo, (HKBD)0);

#if 1 /* turn off for debug */
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	/*  signal(SIGILL,SIG_IGN);  */
	/*  signal(SIGTRAP,SIG_IGN); */
	/*  signal(SIGABRT,SIG_IGN); */
	/*  signal(SIGEMT,SIG_IGN);  */
	/*  signal(SIGFPE,SIG_IGN);  */
	/*  signal(SIGBUS,SIG_IGN);  */
	/*  signal(SIGSEGV,SIG_IGN); */
	/*  signal(SIGSYS,SIG_IGN);  */
	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, SIG_IGN);
	/*  signal(SIGTERM,SIG_IGN); */
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGBREAK, SIG_IGN);
#endif

}

/*
 * EMX shutdown
 */
static void Term_nuke_emx(term *t)
{
	/* Move the cursor to bottom of screen */
	v_gotoxy(0, 23);

	/* Restore the cursor (not necessary) */
	v_ctype(curs_start, curs_end);

	/* Set attribute to gray on black */
	v_attrib(F_WHITE);

	/* Clear the screen */
	v_clear();
}


#ifndef __EMX__CLIENT__

/*
 * Oh no, more prototypes!
 */
static errr CheckEvents(int returnImmediately);
static errr Term_xtra_pipe_emx(int n, int v);
static errr Term_curs_pipe_emx(int x, int y);
static errr Term_wipe_pipe_emx(int x, int y, int n);
static errr Term_text_pipe_emx(int x, int y, int n, unsigned char a, cptr s);
static void Term_init_pipe_emx(term *t);
static void Term_nuke_pipe_emx(term *t);
static FILE *initPipe(char *name);
static int initPipeTerm(term *term, char *name);


/*
 * Check for events -- called by "Term_scan_emx()"
 *
 * Note -- this is probably NOT the most efficient way
 * to "wait" for a keypress (TERM_XTRA_EVENT).
 *
 *
 * This code was ripped from "main-ibm.c" -- consult it to
 * figure out what's going on.
 *
 * See "main-ibm.c" for more information about "macro encoding".
 *
 *
 * The following documentation was cut&pasted from
 * the OS/2 Programming Reference, PRCP.INF
 * <ftp://hobbes.nmsu.edu/pub/os2/dev/16-bit/inf16bit.zip>
-------------------------------------------------------------------------------


 This call returns a character data record from the keyboard.

  KbdCharIn    (CharData, IOWait, KbdHandle)

  CharData (PKBDKEYINFO) - output
     Address of the character data structure:

     asciicharcode (UCHAR)
        ASCII character code. The scan code received from the keyboard is
        translated to the ASCII character code.

     scancode (UCHAR)
        Code received from the keyboard.  The scan code received from the
        keyboard is translated to the ASCII character code.

     status (UCHAR)
        State of the keystroke event:

        Bit       Description

        7-6       00 = Undefined

                  01 = Final character, interim character flag off

                  10 = Interim character

                  11 = Final character, interim character flag on.

        5         1 = Immediate conversion requested.

        4-2       Reserved.

        1         0 = Scan code is a character.

                  1 = Scan code is not a character; is an extended key code
                  from the keyboard.

        0         1 = Shift status returned without character.

     reserved (UCHAR)
        NLS shift status.  Reserved, set to zero.

     shiftkeystat (USHORT)
        Shift key status.

        Bit       Description
        15        SysReq key down
        14        CapsLock key down
        13        NumLock key down
        12        ScrollLock key down
        11        Right Alt key down
        10        Right Ctrl key down
        9         Left Alt key down
        8         Left Ctrl key down
        7         Insert on
        6         CapsLock on
        5         NumLock on
        4         ScrollLock on
        3         Either Alt key down
        2         Either Ctrl key down
        1         Left Shift key down
        0         Right Shift key down

     time (ULONG)
        Time stamp indicating when a key was pressed.  It is specified in
        milliseconds from the time the system was started.

  IOWait (USHORT) - input
     Wait if a character is not available.

     Value     Definition
     0         Requestor waits for a character if one is not available.
     1         Requestor gets an immediate return if no character is
               available.

  KbdHandle (HKBD) - input
     Default keyboard or the logical keyboard.

  rc (USHORT) - return
     Return code descriptions are:

     0         NO_ERROR
     375       ERROR_KBD_INVALID_IOWAIT
     439       ERROR_KBD_INVALID_HANDLE
     445       ERROR_KBD_FOCUS_REQUIRED
     447       ERROR_KBD_KEYBOARD_BUSY
     464       ERROR_KBD_DETACHED
     504       ERROR_KBD_EXTENDED_SG

  Remarks

  On an enhanced keyboard, the secondary enter key returns the normal
  character 0DH and a scan code of E0H.

  Double-byte character codes (DBCS) require two function calls to obtain
  the entire code.

  If shift report is set with KbdSetStatus, the CharData record returned
  reflects changed shift information only.

  Extended ASCII codes are identified with the status byte, bit 1 on and the
  ASCII character code being either 00H or E0H. Both conditions must be
  satisfied for the character to be an extended keystroke.  For extended
  ASCII codes, the scan code byte returned is the second code (extended
  code). Usually the extended ASCII code is the scan code of the primary
  key that was pressed.

  A thread in the foreground session that repeatedly polls the keyboard
  with KbdCharIn (with no wait), can prevent all regular priority class
  threads from executing.  If polling must be used and a minimal amount of
  other processing is being performed, the thread should periodically yield to
  the CPU by issuing a DosSleep call for an interval of at least 5
  milliseconds.


  Family API Considerations

  Some options operate differently in the DOS mode than in the OS /2 mode.
  Therefore, the following restrictions apply to KbdCharIn when coding in
  the DOS mode:

  o The CharData structure includes everything except the time stamp.
  o Interim character is not supported
  o Status can be 0 or 40H
  o KbdHandle is ignored.


-------------------------------------------------------------------------------


   typedef struct _KBDKEYINFO {   / * kbci * /
     UCHAR    chChar;             / * ASCII character code * /
     UCHAR    chScan;             / * Scan Code * /
     UCHAR    fbStatus;           / * State of the character * /
     UCHAR    bNlsShift;          / * Reserved (set to zero) * /
     USHORT   fsState;            / * State of the shift keys * /
     ULONG    time;               / * Time stamp of keystroke (ms since ipl) * /
   }KBDKEYINFO;

   #define INCL_KBD

   USHORT  rc = KbdCharIn(CharData, IOWait, KbdHandle);

   PKBDKEYINFO      CharData;      / * Buffer for data * /
   USHORT           IOWait;        / * Indicate if wait * /
   HKBD             KbdHandle;     / * Keyboard handle * /

   USHORT           rc;            / * return code * /


-------------------------------------------------------------------------------
 *
 */
static errr CheckEvents(int returnImmediately)
{
	int i, k, s;

	bool mc = FALSE;
	bool ms = FALSE;
	bool ma = FALSE;

	/* start OS/2 specific section */

	struct _KBDKEYINFO keyinfo;	/* see structure description above */

	/* Check (and possibly wait) for a keypress */
	/* see function description above */
	KbdCharIn( &keyinfo, returnImmediately, (HKBD)0 );

#if 0
	printf("AC:%x SC:%x ST:%x R1:%x SH:%x TI:%ld\n",	/* OS/2 debug */
		keyinfo.chChar,
		keyinfo.chScan,
		keyinfo.fbStatus,
		keyinfo.bNlsShift,
		keyinfo.fsState,
		keyinfo.time );
#endif

	/* If there wasn't a key, leave now. */
	if ((keyinfo.fbStatus & 0xC0) == 0) return(1);


	/* by a set of lucky coincidences, the data maps directly over. */
	k = keyinfo.chChar;
	s = keyinfo.chScan;
	i = (keyinfo.fsState & 0xFF);

	/* end OS/2 specific section */


	/* Process "normal" keys */
	if ( k != 0 && ((s <= 58) || (s == 0xE0)) )	/* Tweak: allow for ALT-keys */
	{
		/* Enqueue it */
		Term_keypress(k);

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
 * Do a special thing (beep, flush, etc)
 */
static errr Term_xtra_emx(int n, int v)
{
	switch (n)
	{
		case TERM_XTRA_SHAPE:
		if (v)
		{
			v_ctype(curs_start, curs_end);
		}
		else
		{
			v_hidecursor();
		}
		return (0);

		case TERM_XTRA_NOISE:
		DosBeep(440, 50);
		return (0);

		case TERM_XTRA_FLUSH:
		while (!CheckEvents(TRUE));
		return 0;

		case TERM_XTRA_EVENT:

		/* Process an event */
		return (CheckEvents(!v));

		/* Success */
		return (0);

		case TERM_XTRA_CLEAR:
		v_clear();
		return (0);
	}

	return (1);
}

static errr Term_xtra_pipe_emx(int n, int v)
{
	termData *tp=(termData *)Term->data;

	switch (n)
	{
		case TERM_XTRA_NOISE:
		DosBeep(440, 50);
		return (0);

		case TERM_XTRA_SHAPE:
		return (0);

		case TERM_XTRA_EVENT:
		return (CheckEvents(FALSE));

		case TERM_XTRA_CLEAR:

		if (!tp->out) return -1;

		fputc(PIP_XTRA, tp->out);
		fwrite(&n, sizeof(n), 1, tp->out);
		fwrite(&v, sizeof(v), 1, tp->out);
		fflush(tp->out);

		return (0);
	}

	return (1);
}

static errr Term_curs_pipe_emx(int x, int y)
{
	termData *tp=(termData *)Term->data;

	if (!tp->out) return -1;

	fputc(PIP_CURS, tp->out);
	fwrite(&x, sizeof(x), 1, tp->out);
	fwrite(&y, sizeof(y), 1, tp->out);
	fflush(tp->out);

	return (0);
}


static errr Term_wipe_pipe_emx(int x, int y, int n)
{
	termData *tp=(termData *)Term->data;

	if (!tp->out) return -1;

	fputc(PIP_WIPE, tp->out);
	fwrite(&x, sizeof(x), 1, tp->out);
	fwrite(&y, sizeof(y), 1, tp->out);
	fwrite(&n, sizeof(n), 1, tp->out);
	fflush(tp->out);

	return (0);
}


static errr Term_text_pipe_emx(int x, int y, int n, unsigned char a, cptr s)
{
	termData *tp=(termData *)Term->data;

	if (!tp->out) return -1;

	fputc(PIP_TEXT, tp->out);
	fwrite(&x, sizeof(x), 1, tp->out);
	fwrite(&y, sizeof(y), 1, tp->out);
	fwrite(&n, sizeof(n), 1, tp->out);
	fwrite(&a, sizeof(a), 1, tp->out);
	fwrite(s, n, 1, tp->out);
	fflush(tp->out);

	return (0);
}


static void Term_init_pipe_emx(term *t)
{
	termData *tp=(termData *)t->data;

	if (tp->out)
	{
		fputc(PIP_INIT, tp->out);
		fflush(tp->out);
	}
}


static void Term_nuke_pipe_emx(term *t)
{
	termData *tp=(termData *)t->data;

	if (tp->out)
	{
		fputc(PIP_NUKE, tp->out); /* Terminate client */
		fflush(tp->out);
		fclose(tp->out);         /* Close Pipe */
  		FREE(t->data);           /* free xtra memory for pipe data */
		t->data=NULL;            /* Paranoia */
	}
}

static int initPipeTerm(term *t, char *name)
{
   	termData *pipe;
    	int lineCount = 0;
     	char linesin[3] = { "00" };
      size_t bytesin = 0;

	/* get memory for pipe */
 	pipe = RNEW(termData);
	if ((pipe->out=initPipe(name))!=NULL)
	{
    		_sleep2(500);		/* sleep a bit */
    		bytesin = fread(linesin, 2, 1, pipe->out);

      	lineCount = atoi(linesin);
#if 0
		printf("%d blocks, string = \"%s\", count=%d, feof=%d, ferror=%d.\n",bytesin,
  			linesin, lineCount, feof(pipe->out), ferror(pipe->out));
#endif

		/* sanity check: 0 < lines < 99 */
  		if (lineCount < 0 || lineCount > 99 || bytesin < 1) {
       		return 0;
         		/* the window will be idle */
        	}

         	/* reset the stream for writing */
          	fseek(pipe->out, 0, SEEK_CUR);

		/* Initialize the term */
		term_init(t, 80, lineCount, 1);

		/* Special hooks */
		t->init_hook = Term_init_pipe_emx;
		t->nuke_hook = Term_nuke_pipe_emx;

		/* Add the hooks */
		t->text_hook = Term_text_pipe_emx;
		t->wipe_hook = Term_wipe_pipe_emx;
		t->curs_hook = Term_curs_pipe_emx;
		t->xtra_hook = Term_xtra_pipe_emx;

	  	t->data = pipe;
 	  	t->data_flag = TRUE;

		/* Activate it */
		Term_activate(t);
	}
 	return lineCount;
}


const char help_emx[] = "OS/2 EMX (IBM-PC)";


/*
 * Prepare "z-term.c" to use "USE_EMX" built-in video library
 */
errr init_emx(int argc, char **argv)
{
	int i;

	term *t;
 	const char *name;

	/* Unused parameters */
	(void)argc;
	(void)argv;

	/* Initialize the pipe windows */
	for (i = MAX_TERM_DATA-1; i > 0; --i)
	{
		name = angband_term_name[i];
  		/* if client connect successful,  add term to list */
		if (initPipeTerm(&emxterm[i], name))
  			angband_term[i] = &emxterm[i];
	}

	/* Initialize main window */
	t = &emxterm[0];

	/* Initialize the term -- big key buffer */
	term_init(t, 80, 24, 1024);

	/* Special hooks */
	t->init_hook = Term_init_emx;
	t->nuke_hook = Term_nuke_emx;

	/* Add the hooks */
	t->text_hook = Term_text_emx;
	t->wipe_hook = Term_wipe_emx;
	t->curs_hook = Term_curs_emx;
	t->xtra_hook = Term_xtra_emx;

	/* Activate it */
	Term_activate(t);

	/* Assign to angband_term[0] */
 	angband_term[0] = t;

	/* Success */
	return (0);
}

static FILE *initPipe(char *name)
{
	char buf[256];
	FILE *fi;

	sprintf(buf, "\\pipe\\angband\\%s", name);   /* Name of pipe */
	fi=fopen(buf, "r+b");                        /* Look for client */
	return fi;
}

#else /* __EMX__CLIENT__ */

void moveClientWindow(int x, int y)
{
	SWCNTRL sw_info;
	int emxpid;
	BOOL rc;
	HSWITCH swhandle;

	emxpid = getpid();
	swhandle = WinQuerySwitchHandle(NULLHANDLE, emxpid);
	if (swhandle != NULLHANDLE) {
		rc = WinQuerySwitchEntry(swhandle, &sw_info);
		if (rc == 0)
 			WinSetWindowPos(sw_info.hwnd, 0, x, y, 0, 0, SWP_MOVE);
	}
}

int main(int argc, char **argv)
{
	int c, end = 0, lines = 25;
	int x, y, h, n, v;

	FILE *in=NULL;
	char a;
	char buf[160];
	HPIPE pipe;
	APIRET rc;

	/* Check command line */
	if (argc<2 || argc>5 || argc==4)
	{
		printf("Usage: %s Term-1|...|Term-7 [number of lines] [x-position y-position]\n"
		       "Start this before angband.exe\n", argv[0]);
		exit(1);
	}

	if (argc>2) lines = atoi(argv[2]);
	if (lines <= 0) lines = 25;
 	if (argc==5) {
     		x=atoi(argv[3]);
       	y=atoi(argv[4]);
      }

	printf("Looking for Angband... press ^C to abort\n");

	sprintf(buf, "\\pipe\\angband\\%s", argv[1]);

	do
	{
		rc=DosCreateNPipe((PSZ)buf,          /* Create pipe */
		                  &pipe,
		                  NP_ACCESS_DUPLEX,
		                  NP_WAIT|NP_TYPE_BYTE|NP_READMODE_BYTE|1,
		                  2,                 /* Output buffer */
		                  2048,              /* Input buffer */
		                  -1);

		if (rc)                              /* Pipe not created */
		{
			printf("DosCreateNPipe: rc=%ld, pipe=%ld\n", (long)rc, (long)pipe);
			break;
		}

		do
		{
			rc=DosConnectNPipe(pipe);        /* Wait for angband to connect */
			if (!rc) break;
			_sleep2(200);                    /* Sleep for 0.2s  */
		} while (_read_kbd(0, 0, 0)==-1);      /* Until key pressed */

		if (rc) break;

		h=_imphandle(pipe);                  /* Register handle with io */
		setmode(h, O_BINARY);                 /* Make it binary */
		in=fdopen(h, "r+b");                  /* Register handle with stdio */

	} while (0);           /* We don't need no stinking exception handling <g> */

	if (!in)
	{
		printf("Sorry, the pipe connection to Angband could not be established.\n");
		exit(1);
	}

	printf("Connected.\n");

	sprintf(buf, "mode co80,%d", lines);
	system(buf);

	/* now position it */
 	if (argc == 5)
		moveClientWindow(x,y);

	/* send screen length - ONLY time we write to this pipe. */
 	sprintf(buf,"%02d",lines);
  	fwrite(buf,2,1,in);

	/* fseek so that we can start reading. */
 	fseek(in,0,SEEK_CUR);

	/* Infinite loop */
	while (!end)
	{
		/* Get command */
		c = fgetc(in);

		switch (c)
		{
			case PIP_XTRA:
			if (!fread(&n, sizeof(x), 1, in) ||
			    !fread(&v, sizeof(y), 1, in))
				abort();

			/* This hack prevents another hack */
			switch (n)
			{
				case TERM_XTRA_CLEAR:
				v_clear();
				break;

				default:
				printf("Sorry, angband.exe and aclient.exe don't fit together.\n");
				exit(1);
			}

			break;

			case PIP_CURS:
			if (!fread(&x, sizeof(x), 1, in) ||
			    !fread(&y, sizeof(y), 1, in))
				abort();
			Term_curs_emx(x, y);
			break;

			case PIP_WIPE:
			if (!fread(&x, sizeof(x), 1, in) ||
			    !fread(&y, sizeof(y), 1, in) ||
			    !fread(&n, sizeof(n), 1, in))
				abort();
			Term_wipe_emx(x, y, n);
			break;

			case PIP_TEXT:
			if (!fread(&x, sizeof(x), 1, in) ||
			    !fread(&y, sizeof(y), 1, in) ||
			    !fread(&n, sizeof(n), 1, in) ||
			    !fread(&a, sizeof(a), 1, in) || (n > 160) ||
			    !fread(buf, n, 1, in))
				abort();
			Term_text_emx(x, y, n, a, buf);
			break;

			case PIP_INIT:
			Term_init_emx(NULL);
			break;

			case PIP_NUKE:
			case EOF:
			default:
#if 0
			printf("feof=%d ferror=%d.\n",feof(in),ferror(in));
#endif
			Term_nuke_emx(NULL);
			end=1;
			break;
		}
	}

	return 0;
}

#endif /* __EMX__CLIENT__ */

#else /* EMXPM */

void emx_endPM(const char *reason);
int emx_options(char **ANGBAND_DIR_USER,
                char **ANGBAND_DIR_SAVE,
                char **ANGBAND_DIR_INFO,
                char *arg_force_roguelike,
                char *arg_force_original,
                char *arg_fiddle,
                char *arg_wizard,
                char player_name[32]);

void emx_init_window(void **instance, void *main_instance, int n);

errr emx_curs(void *instance, int x, int y);
errr emx_wipe(void *instance, int x, int y, int n);
errr emx_text(void *instance, int x, int y, int n, unsigned char a, cptr s);
void emx_init(void *instance);
void emx_nuke(void *instance);
int emx_read_kbd(void *instance, int wait);
void emx_clear(void *instance);
void emx_hidecursor(void *instance);
void emx_showcursor(void *instance);

/*
 * Type to hold extra data.  Again,  we use termData type (as per pipes)
 */

typedef struct
{
	void *instance;                          /* Pointer to window */
} termData;

/*
 * Display a cursor, on top of a given attr/char
 */
static errr Term_curs_emx(int x, int y)
{
	return emx_curs(((termData *)Term->data)->instance, x, y);
}

/*
 * Erase a grid of space (as if spaces were printed)
 */
static errr Term_wipe_emx(int x, int y, int n)
{
	return emx_wipe(((termData *)Term->data)->instance, x, y, n);
}

/*
 * Draw some text, wiping behind it first
 */
static errr Term_text_emx(int x, int y, int n, unsigned char a, cptr s)
{
	return emx_text(((termData *)Term->data)->instance, x, y, n, a, s);
}

/*
 * EMX initialization
 */
static void Term_init_emx(term *t)
{
	emx_init(((termData *)t->data)->instance);
}

/*
 * EMX shutdown
 */
static void Term_nuke_emx(term *t)
{
	if (t->data_flag && t->data != NULL) {
 		FREE(t->data);
   		t->data = NULL;
    	}
}

/*
 * Oh no, more prototypes!
 */
static errr CheckEvents(int returnImmediately);


/*
 * Check for events -- called by "Term_scan_emx()"
 */
static errr CheckEvents(int returnImmediately)
{
	/* Get key - Macro triggers are generated by emx_read_kbd() */
	int k=emx_read_kbd(((termData *)Term->data)->instance, returnImmediately?0:1);

	/* Nothing ready */
	if (k < 0) return (1);

	/* Enqueue the key */
	Term_keypress(k);

	/* Success */
	return (0);
}

/*
 * Do a special thing (beep, flush, etc)
 */
static errr Term_xtra_emx(int n, int v)
{
	void *instance=((termData *)Term->data)->instance;

	switch (n)
	{
		case TERM_XTRA_SHAPE:
		if (v)
		{
			emx_showcursor(instance);
		}
		else
		{
			emx_hidecursor(instance);
		}
		return (0);

		case TERM_XTRA_NOISE:
		DosBeep(440, 50);
		return (0);

		case TERM_XTRA_FLUSH:
		while (!CheckEvents(TRUE));
		return 0;

		case TERM_XTRA_EVENT:
		return (CheckEvents(!v));

		case TERM_XTRA_CLEAR:
		emx_clear(instance);
		return (0);

		case TERM_XTRA_DELAY:
		if (v > 0) _sleep2(v);
		return (0);
	}

	return (1);
}

void emx_init_term(term *t, void *main_instance, int n)
{
   	termData *inst;

	/* get memory for instance */
 	inst = RNEW(termData);

	t->data = inst;
 	t->data_flag = TRUE;

	/* Initialize window */
	emx_init_window(t->data, main_instance, n);

	/* Initialize the term -- big key buffer */
	term_init(t, 80, 24, 1024);

	/* Special hooks */
	t->init_hook = Term_init_emx;
	t->nuke_hook = Term_nuke_emx;

	/* Add the hooks */
	t->text_hook = Term_text_emx;
	t->wipe_hook = Term_wipe_emx;
	t->curs_hook = Term_curs_emx;
	t->xtra_hook = Term_xtra_emx;
}


/*
 * Prepare "z-term.c" to use "USE_EMX" built-in faked video library
 */
errr init_emx(int argc, char **argv)
{
	int i;


	/* Unused parameters */
	(void)argc;
	(void)argv;

	/* Initialize the windows */
	emx_init_term(&emxterm[0],  NULL, 0);

	for (i = 1; i < MAX_TERM_DATA; ++i)
	{
		emx_init_term(&emxterm[i], emxterm[0].data, i);
	}

	/* Activate main window */
	Term_activate(&emxterm[0]);

	angband_term[0] = &emxterm[0];

	/* Success */
	return (0);
}

static void init_stuff(void)
{
	char path[1024];
	cptr tail;

	/* Get the environment variable */
	tail = getenv("ANGBAND_PATH");

	/* Use the angband_path, or a default */
	strcpy(path, tail ? tail : DEFAULT_PATH);

	/* Hack -- Add a path separator (only if needed) */
	if (!suffix(path, PATH_SEP)) my_strcat(path, PATH_SEP, sizeof(path));

	/* Initialize */
	init_file_paths(path);
}

static void quit_hook(cptr s)
{
	int i;

	for (i = MAX_TERM_DATA - 1; i >= 0; --i)
	{
		/* Shut down the term windows */
		if (angband_term[i])
		{
			term_nuke(angband_term[i]);
			emx_nuke(angband_term[i]->data);
		}
	}

	/* Shut down window system - doesn't return */
	emx_endPM(s);
}


void angbandThread(void *arg)
{
	bool new_game = FALSE;

	int show_score = 0;

	char player_name[32];

	/* Save the "program name" */
	argv0 = (char*)arg;

	/* Use the "main-emx.c" support */
	init_emx(0, NULL);
	ANGBAND_SYS = "ibm";

	/* Get the file paths */
	init_stuff();

	if (!emx_options((char**)&ANGBAND_DIR_USER,
	                 (char**)&ANGBAND_DIR_SAVE,
	                 (char**)&ANGBAND_DIR_INFO,
	                 &arg_force_roguelike,
	                 &arg_force_original,
	                 &arg_fiddle,
	                 &arg_wizard,
	                 player_name)) quit(NULL);

	/* XXX XXX XXX (?) */
	strcpy(op_ptr->full_name, player_name);

	/* Process the player name */
	process_player_name(TRUE);

	/* Tell "quit()" to call "Term_nuke()" */
	quit_aux = quit_hook;

	/* If requested, display scores and quit */
	if (show_score > 0) display_scores(0, show_score);

	/* Catch nasty signals */
	signals_init();

	/* Initialize */
	init_angband();

	/* Wait for response */
	pause_line(Term->hgt - 1);

	/* Play the game */
	play_game(new_game);

	/* Quit */
	quit(NULL);
}

#endif /* EMXPM */

#endif /* USE_EMX */

/*
 * Local Variables:
 * comment-column: 45
 * End:
 *
 */

