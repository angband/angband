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


#include "angband.h"


#ifdef USE_VCS

#include <sys/ioctl.h>
#include <termios.h>


static int fd_vcsa;
static unsigned char *screen;

static int s_width, s_height;
static int cx, cy;


typedef struct term_data
{
	term t;

	int x0,y0,x1,y1,sx,sy;
	unsigned char *base;
} term_data;

#define MAX_VCS_TERM 6

static term_data data[MAX_VCS_TERM];


#if 0
static unsigned char attr_for_color[16]=
{0x00,0x07,0x03,0x0c,0x04,0x02,0x01,0x06,
 0x08,0x0b,0x05,0x0e,0x0c,0x0a,0x09,0x0e
};

static void set_colors(void)
{
}
#else

#if 0
static unsigned char attr_for_color[16]=
{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
 0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
};

static int reverse_linux_color_table[16]=
/*0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 */
{ 0, 4, 2, 6, 1, 5, 3, 7, 8,12,10,14, 9,13,11,15};
#else
static unsigned char attr_for_color[16]=
{0x00,0x07,0x02,0x03,0x04,0x05,0x06,0x01,
 0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
};

static int reverse_linux_color_table[16]=
/*0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 */
{ 0, 7, 2, 6, 1, 5, 3, 4, 8,12,10,14, 9,13,11,15};
#endif

static void set_colors(void)
{
	int i;
	
	for (i = 0; i < 16; i++)
	{
		printf("\033]P%c%02x%02x%02x",
			hexsym[reverse_linux_color_table[i]],
			angband_color_table[i][1],
			angband_color_table[i][2],
			angband_color_table[i][3]);
	}
	fflush(stdout);
}
#endif




static struct termios  norm_termios;
static struct termios  game_termios;


static void keymap_norm(void)
{
	fcntl(0, F_SETFL, (~O_NONBLOCK) & fcntl(0, F_GETFL));
	tcsetattr(0, TCSAFLUSH, &norm_termios);
	printf("\033]R");
	fflush(stdout);
}


static void keymap_game(void)
{
	fcntl(0, F_SETFL, O_NONBLOCK | fcntl(0, F_GETFL));
	tcsetattr(0, TCSAFLUSH, &game_termios);
	set_colors();
}


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
				FD_ZERO(&s);
				FD_SET(0, &s);
				select(1, &s, NULL, NULL, NULL);
			}

			lch = -1;

			while (read(0, &ch, 1) == 1)
			{
				if (lch != -1) Term_keypress(lch);
				lch = ch;
			}

			if (lch != -1)
			{
				if (lch == '\033')
					Term_keypress('`');
				else
					Term_keypress(lch);
			}

			return (0);
		}

		case TERM_XTRA_FLUSH:
		{
			unsigned char ch;
			while (read(0, &ch, 1) == 1);
			return (0);
		}

		case TERM_XTRA_CLEAR:
		{
			int y, x;
			unsigned char *c;

			for (c = td->base, y = td->sy; y; y--, c += 2 * (s_width - td->sx))
			{
				for (x = td->sx; x; x--)
				{
					*c++ = ' ';
					*c++ = 0x07;
				}
			}

			return (0);
		}

		case TERM_XTRA_FRESH:
		{
			lseek(fd_vcsa, 4, SEEK_SET);
			write(fd_vcsa, screen, 2 * s_width * s_height);

#if 0
			fwrite(screen, 1, 2 * s_width * s_height, fvcsa);
#endif /* 0 */

			return (0);
		}

		case TERM_XTRA_NOISE:
		{
			write(1, "\007", 1);
			return (0);
		}

		case TERM_XTRA_ALIVE:
		{
			if (!v)
				keymap_norm();
			else
				keymap_game();
			return (0);
		}

		case TERM_XTRA_DELAY:
		{
			usleep(v * 1000);
			return (0);
		}

		case TERM_XTRA_REACT:
		{
			set_colors();
			return 0;
		}
	}

	/* Unknown or Unhandled action */
	return (1);
}


static errr Term_curs_vcs(int x, int y)
{
	term_data *td = (term_data*)(Term->data);

	cx = x + td->x0;
	cy = y + td->y0;

#if 0
	fseek(fvcsa, 2, SEEK_SET);
	fwrite(&cy, 1, 1, fvcsa);
	fwrite(&cx, 1, 1, fvcsa);
	fflush(fvcsa);
#endif /* 0 */

	lseek(fd_vcsa, 2, SEEK_SET);
	write(fd_vcsa, &cx, 1);
	write(fd_vcsa, &cy, 1);

	return 0;
}


static errr Term_wipe_vcs(int x, int y, int n)
{
	term_data *td = (term_data*)(Term->data);
	unsigned char *c;

	c = &td->base[2 * (s_width * y + x)];

	for (; n; n--)
	{
		*c++ = ' ';
		*c++ = 0x07;
	}

	return 0;
}


static errr Term_text_vcs(int x, int y, int n, byte a, const char *cp)
{
	term_data *td = (term_data*)(Term->data);
	unsigned char *c, col;

	col = attr_for_color[a & 0xf];
	c = &td->base[2 * (s_width * y + x)];

	for (; n; n--)
	{
		*c++ = *cp++;
		*c++ = col;
	}

	return 0;
}


static int active;

static void Term_init_vcs(term *t)
{
	if (active++) return;
	keymap_game();
}


static void Term_nuke_vcs(term *t)
{
	int i;
	unsigned char *c;

	if (--active) return;

	for (i = 0, c = screen; i < s_width; i++)
	{
		*c++ = ' ';
		*c++ = 0x07;
	}

#if 0
	fseek(fvcsa,4+2*s_width*(s_height-1),SEEK_SET);
	fwrite(screen,1,s_width*2,fvcsa);
	free(screen);
	fseek(fvcsa,2,SEEK_SET);
	cx=0;
	cy=s_height-1;
	fwrite(&cy,1,1,fvcsa);
	fwrite(&cx,1,1,fvcsa);
	fclose(fvcsa);
#endif /* 0 */

	lseek(fd_vcsa, 4 + 2 * s_width * (s_height - 1), SEEK_SET);
	write(fd_vcsa, screen, s_width * 2);
	lseek(fd_vcsa, 2, SEEK_SET);
	cx = 0;
	cy = s_height - 1;
	write(fd_vcsa, &cx, 1);
	write(fd_vcsa, &cy, 1);
	close(fd_vcsa);
	
	keymap_norm();
}


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

	t->soft_cursor = TRUE;
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


errr init_vcs(void)
{
	int i;
	unsigned char *c;

	{
		char buf[256];
		c = ttyname(0);

		if (sscanf(c, "/dev/tty%i", &i) != 1)
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

	s_width = s_height = 0;
	read(fd_vcsa, &s_height, 1);
	read(fd_vcsa, &s_width, 1);

#if 0
	fread(&s_height,1,1,fvcsa);
	fread(&s_width,1,1,fvcsa);
#endif /* 0 */

	screen = malloc(s_height * s_width * 2);

	if (!screen)
	{
		fprintf(stderr, "vcsa: out of memory\n");
		return 1;
	}

	for (c = screen, i = 0; i < s_width * s_height; i++)
	{
		*c++ = ' ';
		*c++ = 0x17;
	}

	/* Acquire the current mapping */
	tcgetattr(0, &norm_termios);
	tcgetattr(0, &game_termios);

	game_termios.c_lflag &= ~(ICANON | ECHO | TOSTOP);

#if 0
	/* Force "Ctrl-C" to interupt */
	game_termios.c_cc[VINTR] = (char)3;

	/* Force "Ctrl-Z" to suspend */
	game_termios.c_cc[VSUSP] = (char) /*26*/ -1;

	/* Hack -- Leave "VSTART/VSTOP" alone */
	game_termios.c_cc[VSTART] = (char)-1;
	game_termios.c_cc[VSTOP] = (char)-1;

	/* Disable the standard control characters */
	game_termios.c_cc[VQUIT] = (char)-1;
	game_termios.c_cc[VERASE] = (char)-1;
	game_termios.c_cc[VKILL] = (char)-1;
	game_termios.c_cc[VEOF] = (char)-1;
	game_termios.c_cc[VEOL] = (char)-1;
#endif /* 0 */

	term_data_link(0, 0, 0, 80, 24);

	term_data_link(1, 0, 25, 65, 13);
	term_data_link(2, 81, 0, 51, 24);
	term_data_link(3, 66, 25, 66, 24);
	term_data_link(4, 0, 39, 65, 21);
	term_data_link(5, 66, 50, 66, 10);

	/* Success */
	return 0;
}

#endif /* USE_VCS */
