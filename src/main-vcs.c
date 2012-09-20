/* File: main-vcs.c */

/*
 * Copyright 2001 Alexander Malmberg
 */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */


/*
 * This module uses /dev/vcsa* to write characters and attributes
 * directly to console memory.  /dev/vcsa*'s first 4 bytes are number
 * of lines and columns on the screen, and X and Y position of the
 * cursor with (0,0) as top left.  Then follows lines*columns pairs
 * of (character, attribute) with the contents of the screen.
 */


#include "angband.h"


#ifdef USE_VCS

#include "main.h"

#include <sys/ioctl.h>
#include <termios.h>

#define VCSA_CURSOR	2	/* seek offset of cursor position */
#define VCSA_SCREEN	4	/* seek offset of screen contents */

static void leave_vcs(void);

static int fd_vcsa;
static unsigned char *screen;

static byte s_width, s_height;
static byte cursor[2];


typedef struct term_data
{
	term t;

	int x0,y0,x1,y1,sx,sy;
	unsigned char *base;
} term_data;

#define MAX_VCS_TERM 1

static term_data data[MAX_VCS_TERM];


static unsigned char attr_for_color[16]=
{0x00,0x07,0x02,0x03,0x04,0x05,0x06,0x01,
 0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
};


#define COLOR_BLANK	0x07    /* attr_for_color[TERM_WHITE] */


static int reverse_linux_color_table[16]=
/*0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 */
{ 0, 7, 2, 6, 1, 5, 3, 4, 8,12,10,14, 9,13,11,15};


static void set_colors(void)
{
	int i;

	for (i = 0; i < (int)N_ELEMENTS(reverse_linux_color_table); i++)	
	{
		/* Set palette color */
		printf("\033]P%c%02x%02x%02x",
			hexsym[reverse_linux_color_table[i]],
			angband_color_table[i][1],
			angband_color_table[i][2],
			angband_color_table[i][3]);
	}
	fflush(stdout);
}


static struct termios  norm_termios;
static struct termios  game_termios;


static void reset_terminal(void)
{
	/* Turn off non-blocking on stdin */
	fcntl(0, F_SETFL, (~O_NONBLOCK) & fcntl(0, F_GETFL));

	/* Reset terminal parameters */
	tcsetattr(0, TCSAFLUSH, &norm_termios);

	/* Reset palette */
	printf("\033]R");
	fflush(stdout);
}


static void setup_terminal(void)
{
	/* Make stdin non-blocking */
	fcntl(0, F_SETFL, O_NONBLOCK | fcntl(0, F_GETFL));

	/* Set terminal parameters */
	tcsetattr(0, TCSAFLUSH, &game_termios);

	/* Set up palette */
	set_colors();
}


/*
 * Handle a "special request"
 */
static errr Term_xtra_vcs(int n, int v)
{
	term_data *td = (term_data*)(Term->data);

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
			int y, x;
			unsigned char *c;

			/* Blank out screen memory */
			for (c = td->base, y = td->sy; y; y--, c += 2 * (s_width - td->sx))
			{
				/* Blank out one line */
				for (x = td->sx; x; x--)
				{
					*c++ = ' ';
					*c++ = COLOR_BLANK;
				}
			}

			return (0);
		}

		case TERM_XTRA_FRESH:
		{
			/* Write screen memory to console (vcsa) file */
			lseek(fd_vcsa, VCSA_SCREEN, SEEK_SET);
			write(fd_vcsa, screen, 2 * s_width * s_height);

			return (0);
		}

		case TERM_XTRA_NOISE:
		{
			/* Bell */
			write(1, "\007", 1);
			return (0);
		}

		case TERM_XTRA_ALIVE:
		{
			if (!v)
			{
				/* Reset terminal and move to last line */
				leave_vcs();
			}
			else
			{
				/* Set up terminal */
				setup_terminal();
			}

			return (0);
		}

		case TERM_XTRA_DELAY:
		{
			/* Delay for some milliseconds */
			usleep(v * 1000);
			return (0);
		}

		case TERM_XTRA_REACT:
		{
			/* Set up colours */
			set_colors();
			return 0;
		}
	}

	/* Unknown or Unhandled action */
	return (1);
}


/*
 * Actually MOVE the hardware cursor
 */
static errr Term_curs_vcs(int x, int y)
{
	term_data *td = (term_data*)(Term->data);

	/* Set cursor position */
	cursor[0] = x + td->x0;
	cursor[1] = y + td->y0;

	/* Write cursor position to console (vcsa) file */
	lseek(fd_vcsa, VCSA_CURSOR, SEEK_SET);
	write(fd_vcsa, cursor, sizeof(cursor));

	return 0;
}


/*
 * Erase a grid of space
 */
static errr Term_wipe_vcs(int x, int y, int n)
{
	term_data *td = (term_data*)(Term->data);
	unsigned char *c;

	c = &td->base[2 * (s_width * y + x)];

	/* Blank out n characters in memory */
	for (; n; n--)
	{
		*c++ = ' ';
		*c++ = COLOR_BLANK;
	}

	return 0;
}


/*
 * Place some text on the screen using an attribute
 */
static errr Term_text_vcs(int x, int y, int n, byte a, const char *cp)
{
	term_data *td = (term_data*)(Term->data);
	unsigned char *c, col;

	col = attr_for_color[a % N_ELEMENTS(attr_for_color)];
	c = &td->base[2 * (s_width * y + x)];

	/* Copy n characters to screen memory */
	for (; n; n--)
	{
		*c++ = *cp++;
		*c++ = col;
	}

	return 0;
}


static int active;

/*
 * Init the /dev/vcsa* system
 */
static void Term_init_vcs(term *t)
{
	if (active++) return;

	/* Set up terminal */
	setup_terminal();
}


/*
 * Nuke the /dev/vcsa* system
 */
static void Term_nuke_vcs(term *t)
{
	if (--active) return;

	/* Reset terminal and move to last line */
	leave_vcs();

	/* Close console file */
	close(fd_vcsa);
}


/*
 * Reset terminal and move to last line
 */
static void leave_vcs(void)
{
	int i;
	unsigned char *c;

	/* Blank out one line in memory */
	for (i = 0, c = screen; i < s_width; i++)
	{
		*c++ = ' ';
		*c++ = COLOR_BLANK;
	}

	/* Write blanked-out line to last line in console (vcsa) file */
	lseek(fd_vcsa, VCSA_SCREEN + 2 * s_width * (s_height - 1), SEEK_SET);
	write(fd_vcsa, screen, s_width * 2);

	/* Set cursor position to last line and write it to console */
	lseek(fd_vcsa, VCSA_CURSOR, SEEK_SET);
	cursor[0] = 0;
	cursor[1] = s_height - 1;
	write(fd_vcsa, cursor, sizeof(cursor));
	
	/* Reset terminal */
	reset_terminal();
}


/*
 * Set up a new console terminal
 */
static void term_data_link(int i, int x0, int y0, int sx, int sy)
{
	term_data *td = &data[i];
	term *t = &td->t;

	td->x0 = x0;
	td->y0 = y0;
	td->sx = sx;
	td->sy = sy;
	td->x1 = x0 + sx - 1;
	td->y1 = y0 + sy - 1;
	td->base = &screen[2 * (x0 + y0 * s_width)];

	/* Initialize the term */
	term_init(t, sx, sy, i ? 0 : 256);

	t->soft_cursor = FALSE;
	t->icky_corner = FALSE;
	t->always_text = TRUE;
	t->never_bored = TRUE;
	t->never_frosh = TRUE;

	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';

	/* Prepare the init/nuke hooks */
	t->init_hook = Term_init_vcs;
	t->nuke_hook = Term_nuke_vcs;

	/* Prepare the template hooks */
	t->xtra_hook = Term_xtra_vcs;
	t->curs_hook = Term_curs_vcs;
	t->wipe_hook = Term_wipe_vcs;
	t->text_hook = Term_text_vcs;

	/* Remember where we came from */
	t->data = (vptr)(td);

	/* Activate it */
	if (!i) Term_activate(t);

	/* Global pointer */
	angband_term[i] = t;
}


const char help_vcs[] = "Linux Virtual Console terminal";


/*
 * Prepare /dev/vcsa* for use by the file "z-term.c"
 */
errr init_vcs(int argc, char** argv)
{
	int i;
	unsigned char *c;

	{
		/* Find and open console (vcsa) file */
		char buf[256];
		c = ttyname(0);

		if (c == NULL || sscanf(c, "/dev/tty%i", &i) != 1)
		{
			fprintf(stderr,"can't find my tty\n");
			return 1;
		}

		sprintf(buf, "/dev/vcsa%i", i);
		fd_vcsa = open(buf, O_RDWR);

		if (fd_vcsa == -1)
		{
			perror(buf);
			return 1;
		}
	}

	/* Read screen lines and columns from console (vcsa) file */
	s_width = s_height = 0;
	read(fd_vcsa, &s_height, 1);
	read(fd_vcsa, &s_width, 1);

	/* Allocate memory matching the console (vcsa) file */
	screen = malloc(s_height * s_width * 2);

	if (!screen)
	{
		fprintf(stderr, "vcsa: out of memory\n");
		close(fd_vcsa);
		return 1;
	}

	/* Fill the screen memory with blanks */
	for (c = screen, i = 0; i < s_width * s_height; i++)
	{
		*c++ = ' ';
		*c++ = COLOR_BLANK;
	}

	/* Acquire the current mapping */
	tcgetattr(0, &norm_termios);
	tcgetattr(0, &game_termios);

	/*
	 * Turn off canonical mode, echo of input characters, and sending of
	 * SIGTTOU to proc.group of backgr. process writing to controlling terminal
	 */
	game_termios.c_lflag &= ~(ICANON | ECHO | TOSTOP);

	term_data_link(0, 0, 0, 80, 24);

	/* Success */
	return 0;
}

#endif /* USE_VCS */
