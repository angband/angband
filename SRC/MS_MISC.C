/* ibmpc/ms_misc.c: MSDOS support code

   Copyright (c) 1989-92 James E. Wilson, Don Kneller

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#ifdef __TURBOC__
#include	<conio.h>
#endif /* __TURBOC__ */
 
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>

#include "config.h"
#include "constant.h"
#include "types.h"
#include "externs.h"

#ifdef MSDOS
#ifndef USING_TCIO
/* We don't want to include curses.h when using the tcio.c file.  */
#include <curses.h>
#endif
#ifdef ANSI
#include "ms_ansi.h"
#endif

#ifdef LINT_ARGS
void exit(int);
static FILE *fopenp(char *, char *, char *);
static	unsigned int ioctl(int ,int , unsigned);
#else
void exit();
#ifdef __TURBOC__
int ioctl(int, int, ...);
#endif
static FILE *fopenp(ARG_CHAR_PTR ARG_COMMA ARG_CHAR_PTR ARG_COMMA ARG_CHAR_PTR);
#endif

extern char *getenv(const char *);

#define PATHLEN	80
char	moriatop[PATHLEN];
char	moriasav[PATHLEN];
int	saveprompt = TRUE;
int	ibmbios;
static	int	rawio;
int8u	floorsym = '.';
int8u	wallsym = '#';

/* UNIX compatability routines */
void user_name(buf)
char *buf;
{
  strcpy(buf, getlogin());
}

char	*
getlogin()
{
	char	*cp;

	if ((cp = getenv("USER")) == NULL)
		cp = "player";
	return cp;
}

#ifndef __TURBOC__
unsigned int
sleep(secs)
int	secs;
{
	time_t	finish_time;

	finish_time = time((long *) NULL) + secs;
	while (time((long *) NULL) < finish_time)
		/* nothing */;
	return 0;
}
#endif

#ifdef OLD
/* This is the old code.  It is not strictly correct; it is retained in
   case the correct code below is not portable.  You will also have to
   change the declarations for these functions in externs.h.  */
void
error(fmt, a1, a2, a3, a4)
char	*fmt;
int	a1, a2, a3, a4;
{
	fprintf(stderr, "ANGBAND error: ");
	fprintf(stderr, fmt, a1, a2, a3, a4);
	(void) sleep(2);
	exit(1);
}

void
warn(fmt, a1, a2, a3, a4)
char	*fmt;
int	a1, a2, a3, a4;
{
	fprintf(stderr, "ANGBAND warning: ");
	fprintf(stderr, fmt, a1, a2, a3, a4);
	(void) sleep(2);
}
#else

#include <stdarg.h>

void
error (char *fmt, ...)
{
  va_list p_arg;

  va_start (p_arg, fmt);
  fprintf (stderr, "ANGBAND error: ");
  vfprintf (stderr, fmt, p_arg);
  sleep (2);
  exit (1);
}

void
warn(char *fmt, ...)
{
  va_list p_arg;

  va_start(p_arg, fmt);
  fprintf(stderr, "ANGBAND warning: ");
  vfprintf(stderr, fmt, p_arg);
  sleep(2);
}
#endif

/* Search the path for a file of name "name".  The directory is
 * filled in with the directory part of the path.
 */
static FILE *
fopenp(name, mode, directory)
char *name, *mode, directory[];
{
	char *dp, *pathp, *getenv(), lastch;
	FILE *fp;

	/* Try the default directory first.  If the file can't be opened,
	 * start looking along the path.
	 */
	fp = fopen (name, mode);
	if (fp) {
		directory[0] = '\0';
		return fp;
	}
	pathp = getenv("PATH");
	while (pathp && *pathp) {
		dp = directory;
		while (*pathp && *pathp != ';')
			lastch = *dp++ = *pathp++;
		if (lastch != '\\' && lastch != '/' && lastch != ':')
			*dp++ = '\\';
		(void) strcpy(dp, name);
		fp = fopen (directory, mode);
		if (fp) {
			*dp = '\0';
			return fp;
		}
		if (*pathp)
			pathp++;
	}
	directory[0] = '\0';
	return NULL;
}

/* Read the configuration.
 */
void
msdos_init()
{
	char	buf[BUFSIZ], *bp, opt[PATHLEN];
	int	arg1, arg2, cnt;
	FILE	*fp;

	buf[0] = '\0';
	bp = ANGBAND_CNF_NAME;
	fp = fopenp(bp, "r", buf);
	(void) strcpy(moriasav, buf);
	(void) strcat(moriasav, ANGBAND_SAV);
	if (fp == NULL) {
		warn("Can't find configuration file `%s'\n", bp);
		return;
	}
	printf("Reading configuration from %s%s\n", buf, bp);
	(void) sleep(1);
	while (fgets(buf, sizeof buf, fp)) {
		if (*buf == '#')
			continue;

		cnt = sscanf(buf, "%s", opt);
		/* Turbo C will return EOF when reading an empty line,
		   MSC will correctly read a NULL character */
		if (cnt == 0 ||
#ifdef __TURBOC__
		    cnt == EOF ||
#endif
		    opt[0] == '\0')
			continue;

		/* Go through possible variables
		 */
		if (strcmpi(opt, "GRAPHICS") == 0) {
			cnt = sscanf(buf, "%*s%d %d\n", &arg1, &arg2);
			if (cnt != 2)
				warn("GRAPHICS did not contain 2 values\n");
			else {
				wallsym = (int8u) arg1;
				floorsym = (int8u) arg2;

				/* Adjust lists that depend on '#' and '.' */
				object_list[OBJ_SECRET_DOOR].tchar = wallsym;
			}
		}
		else if (strcmpi(opt, "SAVE") == 0) {
			cnt = sscanf(buf, "%*s%s", opt);
			if (cnt == 0)
				warn("SAVE option requires a filename\n");
			else {
			        bp = strchr (opt, ';');
				if (bp) {
					*bp++ = '\0';
					if (*bp == 'n' || *bp == 'N')
						saveprompt = FALSE;
				}
				if (opt[0])
					(void) strcpy(moriasav, opt);
			}
		}
		else if (strcmpi(opt, "SCORE") == 0) {
			cnt = sscanf(buf, "%*s%s", opt);
			if (cnt == 0)
				warn("SCORE option requires a filename\n");
			else
				(void) strcpy(moriatop, opt);
		}
		else if (strcmpi(opt, "KEYBOARD") == 0) {
			cnt = sscanf(buf, "%*s%s", opt);
			if (cnt == 0)
				warn("KEYBOARD option requires a value\n");
			else if (strcmpi(opt, "ROGUE") == 0)
				rogue_like_commands = TRUE;
			else if (strcmpi(opt, "VMS") == 0)
				rogue_like_commands = FALSE;
		}
		else if (strcmpi(opt, "IBMBIOS") == 0)
			ibmbios = TRUE;
		else if (strcmpi(opt, "RAWIO") == 0)
			rawio = TRUE;
#ifdef ANSI
		/* Usage: ANSI [ check_ansi [ domoveopt [ tgoto ] ] ]
		 * where check_ansi and domoveopt are "Y"es unless explicitly
		 * set to "N"o.	 Tgoto is "N"o unless set to "Y"es.
		 */
		else if (strcmpi(opt, "ANSI") == 0) {
			cnt=sscanf(buf, "%*s%1s%1s%1s",&opt[0],&opt[1],&opt[2]);
			ansi_prep(cnt < 1 || opt[0] == 'y' || opt[0] == 'Y',
				 cnt < 2 || opt[1] == 'y' || opt[1] == 'Y',
				 cnt >= 3 && (opt[2] == 'y' || opt[2] == 'Y'));
		}
#endif
		else
			warn("Unknown configuration line: `%s'\n", buf);
	}
	fclose(fp);

	/* The only text file has been read.  Switch to binary mode */
}

#include <dos.h>
#define DEVICE	0x80
#define RAW	0x20
#define IOCTL	0x44
#define STDIN	fileno(stdin)
#define STDOUT	fileno(stdout)
#define GETBITS	0
#define SETBITS	1

static unsigned	old_stdin, old_stdout;
#ifndef __TURBOC__
static unsigned ioctl();
#endif

void
msdos_raw() {
	if (!rawio)
		return;
	old_stdin = ioctl(STDIN, GETBITS, 0);
	old_stdout = ioctl(STDOUT, GETBITS, 0);
	if (old_stdin & DEVICE)
		ioctl(STDIN, SETBITS, old_stdin | RAW);
	if (old_stdout & DEVICE)
		ioctl(STDOUT, SETBITS, old_stdout | RAW);
}

void
msdos_noraw() {
	if (!rawio)
		return;
	if (old_stdin)
		(void) ioctl(STDIN, SETBITS, old_stdin);
	if (old_stdout)
		(void) ioctl(STDOUT, SETBITS, old_stdout);
}

#ifndef __TURBOC__
static unsigned int
ioctl(handle, mode, setvalue)
unsigned int setvalue;
{
	union REGS regs;

	regs.h.ah = IOCTL;
	regs.h.al = (unsigned char) mode;
	regs.x.bx = handle;
	regs.h.dl = (unsigned char) setvalue;
	regs.h.dh = 0;			/* Zero out dh */
	intdos(&regs, &regs);
	return (regs.x.dx);
}
#endif

/* Normal characters are output when the shift key is not pushed.
 * Shift characters are output when either shift key is pushed.
 */
#define KEYPADHI	83
#define KEYPADLOW	71
#define ISKEYPAD(x)	(KEYPADLOW <= (x) && (x) <= KEYPADHI)
#undef CTRL
#define CTRL(x)		(x - '@')
typedef struct {
	char normal, shift, numlock;
} KEY;
static KEY roguekeypad[KEYPADHI - KEYPADLOW + 1] = {
	{'y', 'Y', CTRL('Y')},			/* 7 */
	{'k', 'K', CTRL('K')},			/* 8 */
	{'u', 'U', CTRL('U')},			/* 9 */
	{'.', '.', '.'},			/* - */
	{'h', 'H', CTRL('H')},			/* 4 */
	{'.', '.', '.'},			/* 5 */
	{'l', 'L', CTRL('L')},			/* 6 */
	{CTRL('P'), CTRL('P'), CTRL('P')},	/* + */
	{'b', 'B', CTRL('B')},			/* 1 */
	{'j', 'J', CTRL('J')},			/* 2 */
	{'n', 'N', CTRL('N')},			/* 3 */
	{'i', 'i', 'i'},			/* Ins */
	{'.', '.', '.'}				/* Del */
};
static KEY originalkeypad[KEYPADHI - KEYPADLOW + 1] = {
	{'7', '7', '7'},			/* 7 */
	{'8', '8', '8'},			/* 8 */
	{'9', '9', '9'},			/* 9 */
	{'-', '-', '-'},			/* - */
	{'4', '4', '4'},			/* 4 */
	{'5', '5', '5'},			/* 5  - move */
	{'6', '6', '6'},			/* 6 */
	{CTRL('M'), CTRL('M'), CTRL('M')},	/* + */
	{'1', '1', '1'},			/* 1 */
	{'2', '2', '2'},			/* 2 */
	{'3', '3', '3'},			/* 3 */
	{'i', 'i', 'i'},		/* Ins */
	{'.', '.', '.'}			/* Del */
};

/* bios_getch gets keys directly with a BIOS call.
 */
#define SHIFT		(0x1 | 0x2)
#define NUMLOCK		0x20
#define KEYBRD_BIOS	0x16

int
bios_getch()
{
	unsigned char scan, shift;
	int	ch;
	KEY	*kp;
	union REGS regs;

	if (rogue_like_commands)
		kp = roguekeypad;
	else
		kp = originalkeypad;

	/* Get scan code.
	 */
	regs.h.ah = 0;
	int86(KEYBRD_BIOS, &regs, &regs);
	ch = regs.h.al;
	scan = regs.h.ah;

	/* Get shift status.
	 */
	regs.h.ah = 2;
	int86(KEYBRD_BIOS, &regs, &regs);
	shift = regs.h.al;

	/* If scan code is for the keypad, translate it.
	 */
	if (ISKEYPAD(scan)) {
		if (shift & NUMLOCK)
			ch = kp[scan - KEYPADLOW].numlock;
		else if (shift & SHIFT)
			ch = kp[scan - KEYPADLOW].shift;
		else
			ch = kp[scan - KEYPADLOW].normal;
	}
	return ch;
}

int
msdos_getch()
{
	int	ch;

	if (ibmbios)
		ch = bios_getch();
	else {
		ch = getch();
		if (ch == 0)
			ch = getch();
	}
	return ch;
}

#if 0
/* This intro message deleted because it is obsolete.  */

/* Hardcode the introductory message in */
void
msdos_intro()
{
        char buf[80];

	clear_screen();
        wmove(stdscr,0,0);
	waddstr(stdscr,"                         *********************");
	wmove(stdscr,1,0);
        sprintf(buf,"                         **   Moria %d.%d    **",
	  CUR_VERSION_MAJ, CUR_VERSION_MIN);
        waddstr(stdscr,buf);
        wmove(stdscr,2,0);
	waddstr(stdscr,"                         *********************");
        wmove(stdscr,3,0);
	waddstr(stdscr,"                   COPYRIGHT (c) Robert Alan Koeneke");
        wmove(stdscr,5,0);
	waddstr(stdscr,"Programmers : Robert Alan Koeneke / University of Oklahoma");
        wmove(stdscr,6,0);
	waddstr(stdscr,"              Jimmey Wayne Todd   / University of Oklahoma");
        wmove(stdscr,8,0);
	waddstr(stdscr,"UNIX Port   : James E. Wilson     / Cygnus Support");
        wmove(stdscr,10,0);
	waddstr(stdscr,"MSDOS Port  : Don Kneller         / 1349 - 10th ave");
        wmove(stdscr,11,0);
	waddstr(stdscr,
		"                                  / San Francisco, CA 94122");
        wmove(stdscr,12,0);
	waddstr(stdscr,"                                  / Dec 12, 1988");
#ifdef TURBOC_COLOR
		  wmove(stdscr,13,0);
	 waddstr(stdscr,"Color       : Justin Anderson     / Virginia Tech");
#endif
	pause_line(23);
}
#endif

#ifdef PC_CURSES
/* Seems to be a bug in PCcurses whereby it won't really clear the screen
 * if there are characters there it doesn't know about.
 */
#define VIDEOINT	0x10
void
bios_clear()
{
	union REGS regs;
	unsigned char	nocols, activepage;

#ifdef ANSI
	if (ansi)
		return;
#endif

	/* get video attributes */
	regs.h.ah = 15;
	int86(VIDEOINT, &regs, &regs);
	nocols = regs.h.ah;
	activepage = regs.h.bh;

	/* Move to lower right corner */
	regs.h.ah = 2;
	regs.h.dh = (unsigned char) 24;
	regs.h.dl = nocols - 1;	/* lower right col */
	regs.h.bh = activepage;
	int86(VIDEOINT, &regs, &regs);

	/* get current attribute into bh */
	regs.h.ah = 8;
	regs.h.bh = activepage;
	int86(VIDEOINT, &regs, &regs);
	regs.h.bh = regs.h.ah;

	regs.h.cl = 0;	/* upper left row */
	regs.h.ch = 0;	/* upper left col */
	regs.h.dh = (unsigned char) 24;	/* lower right row */
	regs.h.dl = nocols - 1;	/* lower right col */
	regs.h.al = 0;	/* clear window */
	regs.h.ah = 7;	/* scroll down */
	int86(VIDEOINT, &regs, &regs);
}
#endif

#endif
