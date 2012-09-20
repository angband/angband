/* File: main-dos.c */

/*
 * Copyright (c) 1997 Ben Harrison, Robert Ruehlmann, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */


/*
 * This file helps Angband work with DOS computers.
 *
 * Adapted from "main-ibm.c".
 *
 * Author: Robert Ruehlmann (rr9@angband.org).
 * See "http://thangorodrim.angband.org/".
 *
 * Initial framework (and some code) by Ben Harrison (benh@phial.com).
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
 * The "lib/user/pref.prf" file contains macro definitions and possible
 * alternative color set definitions.
 *
 * The "lib/user/font.prf" contains attr/char mappings for use with the
 * special fonts in the "lib/xtra/font/" directory.
 *
 * The "lib/user/graf.prf" contains attr/char mappings for use with the
 * special bitmaps in the "lib/xtra/graf/" directory.
 *
 *
 * Both "shift" keys are treated as "identical", and all the modifier keys
 * (control, shift, alt) are ignored when used with "normal" keys, unless
 * they modify the underlying "ascii" value of the key.  You must use the
 * new "user pref files" to be able to interact with the keypad and such.
 *
 * Note that "Term_xtra_dos_react()" allows runtime color, graphics,
 * screen resolution, and sound modification.
 *
 *
 * The sound code uses *.wav files placed in the "lib/xtra/sound" folder.
 * Every sound-event can have several samples assigned to it and a random
 * one will be played when the event occures. Look at the
 * "lib/xtra/sound/sound.cfg" configuration file for more informations.
 *
 * The background music uses midi-files (and mod files) from the
 * "lib/xtra/music" folder.
 *
 *
 * Comment by Ben Harrison (benh@phial.com):
 *
 * On my Windows NT box, modes "VESA1" and "VESA2B" seem to work, but
 * result in the game running with no visible output.  Mode "VESA2L"
 * clears the screen and then fails silently.  All other modes fail
 * instantly.  To recover from such "invisible" modes, you can try
 * typing escape, plus control-x, plus escape.  XXX XXX XXX
 */

#include "angband.h"


#ifdef USE_DOS

#include <allegro.h>

#ifdef USE_MOD_FILES
#include <jgmod.h>
#endif /* USE_MOD_FILES */

#include <bios.h>
#include <dos.h>
#include <keys.h>
#include <unistd.h>
#include <dir.h>

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

	int number;

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

#ifdef USE_BACKGROUND

	int window_type;

#endif /* USE_BACKGROUND */

};

/*
 * The current screen resolution
 */
static int resolution;

#ifdef USE_BACKGROUND

/*
 * The background images
 */
BITMAP *background[17];

#endif /* USE_BACKGROUND */


/*
 * An array of term_data's
 */
static term_data data[MAX_TERM_DATA];


#ifdef USE_GRAPHICS

/*
 * Are graphics already initialized ?
 */
static bool graphics_initialized = FALSE;

#endif /* USE_GRAPHICS */


/*
 * Small bitmap for the cursor
 */
static BITMAP *cursor;


#ifdef USE_SOUND

/*
 * Is the sound already initialized ?
 */
static bool sound_initialized = FALSE;

# ifdef USE_MOD_FILES
/*
 * Is the mod-file support already initialized ?
 */
static bool mod_file_initialized = FALSE;

# endif /* USE_MOD_FILES */

/*
 * Volume settings
 */
static int digi_volume;
static int midi_volume;

/*
 * The currently playing song
 */
static MIDI *midi_song = NULL;

# ifdef USE_MOD_FILES

static JGMOD *mod_song = NULL;

# endif /* USE_MOD_FILES */

static int current_song;

/*
 * The number of available songs
 */
static int song_number;

/*
 * The maximum number of available songs
 */
#define MAX_SONGS 255

static char music_files[MAX_SONGS][16];

/*
 * The maximum number of samples per sound-event
 */
#define SAMPLE_MAX 10

/*
 * An array of sound files
 */
static SAMPLE* samples[SOUND_MAX][SAMPLE_MAX];

/*
 * The number of available samples for every event
 */
static int sample_count[SOUND_MAX];

#endif /* USE_SOUND */


/*
 *  Extra paths
 */
static char xtra_font_dir[1024];
static char xtra_graf_dir[1024];
static char xtra_sound_dir[1024];
static char xtra_music_dir[1024];

/*
 * List of used videomodes to reduce executable size
 */
BEGIN_GFX_DRIVER_LIST
	GFX_DRIVER_VBEAF
	GFX_DRIVER_VGA
	GFX_DRIVER_VESA3
	GFX_DRIVER_VESA2L
	GFX_DRIVER_VESA2B
	GFX_DRIVER_VESA1
END_GFX_DRIVER_LIST


/*
 * List of used color depths to reduce executeable size
 */
BEGIN_COLOR_DEPTH_LIST
	COLOR_DEPTH_8
END_COLOR_DEPTH_LIST


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
 * Prototypes
 */
static errr Term_xtra_dos_event(int v);
static void Term_xtra_dos_react(void);
static void Term_xtra_dos_clear(void);
static errr Term_xtra_dos(int n, int v);
static errr Term_user_dos(int n);
static errr Term_curs_dos(int x, int y);
static errr Term_wipe_dos(int x, int y, int n);
static errr Term_text_dos(int x, int y, int n, byte a, const char *cp);
static void Term_init_dos(term *t);
static void Term_nuke_dos(term *t);
static void term_data_link(term_data *td);
static void dos_dump_screen(void);
static void dos_quit_hook(cptr str);
static bool init_windows(void);
errr init_dos(void);
#ifdef USE_SOUND
static bool init_sound(void);
static errr Term_xtra_dos_sound(int v);
static void play_song(void);
#endif /* USE_SOUND */
#ifdef USE_GRAPHICS
static bool init_graphics(void);
# ifdef USE_TRANSPARENCY
static errr Term_pict_dos(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp);
# else /* USE_TRANSPARENCY */
static errr Term_pict_dos(int x, int y, int n, const byte *ap, const char *cp);
# endif /* USE_TRANSPARENCY */
#endif /* USE_GRAPHICS */


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
 * combination with the "lib/user/pref.prf" file, to handle most of the
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
 *
 * Addition for the DOS version: Dump-screen function with the
 * "Ctrl-Print" key saves a bitmap with the screen contents to
 * "lib/user/dump.bmp".
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

	/* Dump the screen with "Ctrl-Print" */
	if ((s == 0x72) && mc)
	{
		/* Dump the screen */
		dos_dump_screen();

		/* Success */
		return (0);
	}

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
 * React to global changes in the colors, graphics, and sound settings.
 */
static void Term_xtra_dos_react(void)
{
	int i;

#ifdef USE_SPECIAL_BACKGROUND

	int j;

	term_data *td;

#endif /* USE_SPECIAL_BACKGROUND */

	/*
	 * Set the Angband colors
	 */
	for (i = 0; i < 16; i++)
	{
		RGB color;

		/* Extract desired values */
		char rv = angband_color_table[i][1] >> 2;
		char gv = angband_color_table[i][2] >> 2;
		char bv = angband_color_table[i][3] >> 2;

		/* Set the colors */
		color.r = rv;
		color.g = gv;
		color.b = bv;

		set_color(COLOR_OFFSET + i, &color);
	}

#ifdef USE_GRAPHICS

	/*
	 * Handle "arg_graphics"
	 */
	if (use_graphics != arg_graphics)
	{
		/* Initialize (if needed) */
		if (arg_graphics && !init_graphics())
		{
			/* Warning */
			plog("Cannot initialize graphics!");

			/* Cannot enable */
			arg_graphics = GRAPHICS_NONE;
		}

		/* Change setting */
		use_graphics = arg_graphics;
	}

#endif /* USE_GRAPHICS */

#ifdef USE_SOUND

	/*
	 * Handle "arg_sound"
	 */
	if (use_sound != arg_sound)
	{
		/* Clear the old song */
		if (midi_song) destroy_midi(midi_song);
		midi_song = NULL;

#ifdef USE_MOD_FILES
		if (mod_file_initialized)
		{
			stop_mod();
			destroy_mod(mod_song);
		}
#endif /* USE_MOD_FILES */

		/* Initialize (if needed) */
		if (arg_sound && !init_sound())
		{
			/* Warning */
			plog("Cannot initialize sound!");

			/* Cannot enable */
			arg_sound = FALSE;
		}

		/* Change setting */
		use_sound = arg_sound;
	}

#endif /* USE_SOUND */

#ifdef USE_SPECIAL_BACKGROUND

	/*
	 * Initialize the window backgrounds
	 */
	for (i = 0; i < 8; i++)
	{
		td = &data[i];

		/* Window flags */
		for (j = 0; j < 16; j++)
		{
			if (op_ptr->window_flag[i] & (1L << j))
			{
				if (background[j + 1])
				{
					td->window_type = j + 1;
				}
				else
				{
					td->window_type = 0;
				}
			}
		}
	}
#endif /* USE_SPECIAL_BACKGROUND */
}


/*
 * Clear a terminal
 *
 * Fills the terminal area with black color or with
 * the background image
 */
static void Term_xtra_dos_clear(void)
{
	term_data *td = (term_data*)(Term->data);

#ifdef USE_BACKGROUND
	int bgrnd;
#endif /* USE_BACKGROUND */

	int x1, y1;
	int w1, h1;

	/* Location */
	x1 = td->x;
	y1 = td->y;

	/* Size */
	w1 = td->tile_wid * td->cols;
	h1 = td->tile_hgt * td->rows;

#ifdef USE_BACKGROUND

	bgrnd = td->window_type;

	if (background[bgrnd])
	{
		/* Draw the background */
		stretch_blit(background[bgrnd], screen,
			0, 0, background[bgrnd]->w, background[bgrnd]->h,
			x1, y1, w1, h1);
	}
	else

#endif /* USE_BACKGROUND */

	{
		/* Draw the Term black */
		rectfill(screen,
	        	x1, y1, x1 + w1 - 1, y1 + h1 - 1,
			COLOR_OFFSET + TERM_DARK);
	}
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
			/* Clear the screen */
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

		/* Do something useful if bored */
		case TERM_XTRA_BORED:
		{
#ifdef USE_SOUND
			/*
			 * Check for end of song and start a new one
			 */
			if (!use_sound) return (0);

#ifdef USE_MOD_FILES
			if (song_number && (midi_pos == -1) && !is_mod_playing())
#else /* USE_MOD_FILES */
			if (song_number && (midi_pos == -1))
#endif /* USE_MOD_FILES */
			{
				if (song_number > 1)
				{
					/* Get a *new* song at random */
					while (1)
					{
						n = randint(song_number);
						if (n != current_song) break;
					}
					current_song = n;
				}
				else
				{
					/* We only have one song, so loop it */
					current_song = 1;
				}

				/* Play the song */
				play_song();
			}

#endif /* USE_SOUND */

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

#ifdef USE_SOUND

		/* Make a sound */
		case TERM_XTRA_SOUND:
		{
			return (Term_xtra_dos_sound(v));
		}

#endif /* USE_SOUND */

	}

	/* Unknown request */
	return (1);
}


/*
 * Do a "user action" on the current "term"
 */
static errr Term_user_dos(int n)
{
	int k;

	char status[4];

	char section[80];

	/* Interact */
	while (1)
	{
		/* Clear screen */
		Term_clear();

		/* Print date and time of compilation */
		prt(format("Compiled: %s %s\n", __TIME__, __DATE__), 1, 45);

		/* Why are we here */
		prt("DOS options", 2, 0);

		/* Give some choices */
#ifdef USE_SOUND
		prt("(V) Sound Volume", 4, 5);
		prt("(M) Music Volume", 5, 5);
#endif /* USE_SOUND */

#ifdef USE_GRAPHICS

		if (arg_graphics)
		{
			strcpy(status, "On");
		}
		else
		{
			strcpy(status, "Off");
		}
		prt(format("(G) Graphics : %s", status), 7, 5);

#endif /* USE_GRAPHICS */

#ifdef USE_SOUND

		if (arg_sound)
		{
			strcpy(status, "On");
		}
		else
		{
			strcpy(status, "Off");
		}
		prt(format("(S) Sound/Music : %s", status), 8, 5);

#endif /* USE_SOUND */

		prt("(R) Screen resolution", 12, 5);

		prt("(W) Save current options", 14, 5);

		/* Prompt */
		prt("Command: ", 18, 0);

		/* Get command */
		k = inkey();

		/* Exit */
		if (k == ESCAPE) break;

		/* Analyze */
		switch (k)
		{
#ifdef USE_SOUND
			/* Sound Volume */
			case 'V':
			case 'v':
			{
				/* Prompt */
				prt("Command: Sound Volume", 18, 0);

				/* Get a new value */
				while (1)
				{
					prt(format("Current Volume: %d", digi_volume), 22, 0);
					prt("Change Volume (+, - or ESC to accept): ", 20, 0);
					k = inkey();
					if (k == ESCAPE) break;
					switch (k)
					{
						case '+':
						{
							digi_volume++;
							if (digi_volume > 255) digi_volume = 255;
							break;
						}
						case '-':
						{
							digi_volume--;
							if (digi_volume < 0) digi_volume = 0;
							break;
						}
						/* Unknown option */
						default:
						{
							break;
						}
					}
					set_volume(digi_volume, -1);
				}
				break;
			}

			/* Music Volume */
			case 'M':
			case 'm':
			{
				/* Prompt */
				prt("Command: Music Volume", 18, 0);

				/* Get a new value */
				while (1)
				{
					prt(format("Current Volume: %d", midi_volume), 22, 0);
					prt("Change Volume (+, - or ESC to accept): ", 20, 0);
					k = inkey();
					if (k == ESCAPE) break;
					switch (k)
					{
						case '+':
						{
							midi_volume++;
							if (midi_volume > 255) midi_volume = 255;
							break;
						}
						case '-':
						{
							midi_volume--;
							if (midi_volume < 0) midi_volume = 0;
							break;
						}
						/* Unknown option */
						default:
						{
							break;
						}
					}
					set_volume(-1, midi_volume);
				}
				break;
			}

#endif /* USE_SOUND */

#ifdef USE_GRAPHICS

			/* Switch graphics on/off */
			case 'G':
			case 'g':
			{
				/* Toggle "arg_graphics" */
				arg_graphics = !arg_graphics;

				/* React to changes */
				Term_xtra_dos_react();

				/* Reset visuals */
#ifdef ANGBAND_2_8_1
				reset_visuals();
#else /* ANGBAND_2_8_1 */
				reset_visuals(TRUE);
#endif /* ANGBAND_2_8_1 */
				break;
			}

#endif /* USE_GRAPHICS */

#ifdef USE_SOUND

			/* Sound/Music On/Off */
			case 'S':
			case 's':
			{
				/* Toggle "arg_sound" */
				arg_sound = !arg_sound;

				/* React to changes */
				Term_xtra_dos_react();

				break;
			}

#endif /* USE_SOUND */

			/* Screen Resolution */
			case 'R':
			case 'r':
			{
				int h, w, i = 1;
				char *descr;

				/* Clear screen */
				Term_clear();

				/* Prompt */
				prt("Command: Screen Resolution", 1, 0);
				prt("Restart Angband to get the new screenmode.", 3, 0);

				/* Get a list of the available presets */
				while (1)
				{
					/* Section name */
					sprintf(section, "Mode-%d", i);

					/* Get new values or end the list */
					if (!(w = get_config_int(section, "screen_wid", 0)) || (i == 16)) break;
					h = get_config_int(section, "screen_hgt", 0);

					/* Get a extra description of the resolution */
					descr = get_config_string(section, "Description", "");

					/* Print it */
					prt(format("(%d) %d x %d   %s", i, w, h, descr), 4 + i, 0);

					/* Next */
					i++;
				}

				/* Get a new resolution */
				prt(format("Screen Resolution : %d",resolution), 20, 0);
				k = inkey();
				if (k == ESCAPE) break;
				if (isdigit(k)) resolution = D2I(k);

				/* Check for min, max value */
				if ((resolution < 1) || (resolution >= i)) resolution = 1;

				/* Save the resolution */
				set_config_int("Angband", "Resolution", resolution);

				/* Return */
				break;
			}


			/* Save current option */
			case 'W':
			case 'w':
			{
				prt("Saving current options", 18, 0);

#ifdef USE_SOUND
				set_config_int("sound", "digi_volume", digi_volume);
				set_config_int("sound", "midi_volume", midi_volume);
#endif /* USE_SOUND */
				set_config_int("Angband", "Graphics", arg_graphics);
				set_config_int("Angband", "Sound", arg_sound);

				break;
			}

			/* Unknown option */
			default:
			{
				break;
			}
		}

		/* Flush messages */
		msg_print(NULL);
	}

	/* Redraw it */
	Term_key_push(KTRL('R'));

	/* Unknown */
	return (0);
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

#ifdef USE_BACKGROUND
	int bgrnd;
#endif /* USE_BACKGROUND */

	int x1, y1;
	int w1, h1;

	/* Location */
	x1 = x * td->tile_wid + td->x;
	y1 = y * td->tile_hgt + td->y;

	/* Size */
	w1 = n * td->tile_wid;
	h1 = td->tile_hgt;

#ifdef USE_BACKGROUND

	bgrnd = td->window_type;

	if (background[bgrnd])
	{
		int source_x = x * background[bgrnd]->w / td->cols;
		int source_y = y * background[bgrnd]->h / td->rows;
		int source_w = n * background[bgrnd]->w / td->cols;
		int source_h = background[bgrnd]->h / td->rows;

		/* Draw the background */
		stretch_blit(background[bgrnd], screen,
			source_x, source_y, source_w, source_h,
			x1, y1, w1, h1);
	}
	else

#endif /* USE_BACKGROUND */

	{
		/* Draw a black block */
		rectfill(screen, x1, y1, x1 + w1 - 1, y1 + h1 - 1,
	        	COLOR_OFFSET + TERM_DARK);
	}

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

	unsigned char text[257];

	/* Location */
	x1 = x * td->tile_wid + td->x;
	y1 = y * td->tile_hgt + td->y;

	/* Erase old contents */
	Term_wipe_dos(x, y, n);

#ifdef USE_SPECIAL_BACKGROUND

	/* Show text in black in the message window */
	if (op_ptr->window_flag[td->number] & (PW_MESSAGE)) a = 0;

#endif /* USE_SPECIAL_BACKGROUND */

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
#ifdef USE_TRANSPARENCY
static errr Term_pict_dos(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp)
#else /* USE_TRANSPARENCY */
static errr Term_pict_dos(int x, int y, int n, const byte *ap, const char *cp)
#endif /* USE_TRANSPARENCY */
{
	term_data *td = (term_data*)(Term->data);

	int i;

	int w, h;

	int x1, y1;
	int x2, y2;

# ifdef USE_TRANSPARENCY

	int x3, y3;

# endif /* USE_TRANSPARENCY */

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
		x2 = (cp[i] & 0x7F) * w;
		y2 = (ap[i] & 0x7F) * h;

# ifdef USE_TRANSPARENCY
		x3 = (tcp[i] & 0x7F) * w;
		y3 = (tap[i] & 0x7F) * h;

		/* Blit the tile to the screen */
		blit(td->tiles, screen, x3, y3, x1, y1, w, h);

		/* Blit the tile to the screen */
		masked_blit(td->tiles, screen, x2, y2, x1, y1, w, h);

# else /* USE_TRANSPARENCY */

		/* Blit the tile to the screen */
		blit(td->tiles, screen, x2, y2, x1, y1, w, h);

# endif /* USE_TRANSPARENCY */

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
	t->never_bored = FALSE;

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
	t->user_hook = Term_user_dos;
	t->text_hook = Term_text_dos;

#ifdef USE_GRAPHICS

	/* Prepare the graphics hook */
	t->pict_hook = Term_pict_dos;

	/* Use "Term_pict" for "graphic" data */
	t->higher_pict = TRUE;

#endif /* USE_GRAPHICS */

	/* Remember where we came from */
	t->data = (vptr)(td);
}


/*
 * Shut down visual system, then fall back into standard "quit()"
 */
static void dos_quit_hook(cptr str)
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

#ifdef USE_BACKGROUND

	/* Free the background bitmaps */
	for (i = 0; i < 17; i++)
	{
		if (background[i]) destroy_bitmap(background[i]);
	}

#endif /* USE_BACKGROUND */


#ifdef USE_SOUND

	if (sound_initialized)
	{
		/* Destroy samples */
		for (i = 1; i < SOUND_MAX; i++)
		{
			int j;

			for (j = 0; j < sample_count[i]; j++)
			{
				if (samples[i][j]) destroy_sample(samples[i][j]);
			}
		}
	}

	/* Clear the old song */
	if (midi_song) destroy_midi(midi_song);
	midi_song =NULL;
# ifdef USE_MOD_FILES
	if (mod_file_initialized)
	{
		stop_mod();
		destroy_mod(mod_song);
	}
# endif /* USE_MOD_FILES */

#endif /* USE_SOUND */

	/* Shut down Allegro */
	allegro_exit();
}


/*
 * Dump the screen to "lib/user/dump.bmp"
 */
static void dos_dump_screen(void)
{
	/* Bitmap and palette of the screen */
	BITMAP *bmp;
	PALETTE pal;

	/* Filename */
	char filename[1024];

	/* Get bitmap and palette of the screen */
	bmp = create_sub_bitmap(screen, 0, 0, SCREEN_W, SCREEN_H);
	get_palette(pal);

	/* Build the filename for the screen-dump */
	path_build(filename, 1024, ANGBAND_DIR_USER, "dump.bmp");

	/* Save it */
	save_bmp(filename, bmp, pal);

	/* Free up the memory */
	if (bmp) destroy_bitmap(bmp);

	/* Success message */
	msg_print("Screen dump saved.");
	msg_print(NULL);
}


/* GRX font file reader by Mark Wodrich.
 *
 * GRX FNT files consist of the header data (see struct below). If the font
 * is proportional, followed by a table of widths per character (unsigned
 * shorts). Then, the data for each character follows. 1 bit/pixel is used,
 * with each line of the character stored in contiguous bytes. High bit of
 * first byte is leftmost pixel of line.
 *
 * Note : FNT files can have a variable number of characters, so we must
 *        check that the chars 32..127 exist.
 */

#define FONTMAGIC       0x19590214L


/* .FNT file header */
typedef struct
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
} FNTfile_header;


#define GRX_TMP_SIZE    4096



/* converts images from bit to byte format */
static void convert_grx_bitmap(int width, int height, unsigned char *src, unsigned char *dest)
{
	unsigned short x, y, bytes_per_line;
	unsigned char bitpos, bitset;

	bytes_per_line = (width + 7) >> 3;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			bitpos = 7-(x&7);
			bitset = !!(src[(bytes_per_line * y) + (x >> 3)] & (1 << bitpos));
			dest[y * width + x] = bitset;
		}
	}
}



/* reads GRX format images from disk */
static unsigned char **load_grx_bmps(PACKFILE *f, FNTfile_header *hdr, int numchar, unsigned short *wtable)
{
	int t, width, bmp_size;
	unsigned char *temp;
	unsigned char **bmp;

	/* alloc array of bitmap pointers */
	bmp = malloc(sizeof(unsigned char *) * numchar);

	/* assume it's fixed width for now */
	width = hdr->width;

	/* temporary working area to store FNT bitmap */
	temp = malloc(GRX_TMP_SIZE);

	for (t = 0; t < numchar; t++)
	{
		/* if prop. get character width */
		if (!hdr->isfixed)
			width = wtable[t];

		/* work out how many bytes to read */
		bmp_size = ((width + 7) >> 3) * hdr->height;

		/* oops, out of space! */
		if (bmp_size > GRX_TMP_SIZE)
		{
			free(temp);
			for (t--; t >= 0; t--)
			free(bmp[t]);
			free(bmp);
			return NULL;
		}

		/* alloc space for converted bitmap */
		bmp[t] = malloc(width * hdr->height);

		/* read data */
		pack_fread(temp, bmp_size, f);

		/* convert to 1 byte/pixel */
		convert_grx_bitmap(width, hdr->height, temp, bmp[t]);
	}

	free(temp);
	return bmp;
}



/* main import routine for the GRX font format */
static FONT *import_grx_font(char *fname)
{
	PACKFILE *f;
	FNTfile_header hdr;              /* GRX font header */
	int numchar;                     /* number of characters in the font */
	unsigned short *wtable = NULL;   /* table of widths for each character */
	unsigned char **bmp;             /* array of font bitmaps */
	FONT *font = NULL;               /* the Allegro font */
	FONT_PROP *font_prop;
	int c, c2, start, width;

	f = pack_fopen(fname, F_READ);
	if (!f)
		return NULL;

	pack_fread(&hdr, sizeof(hdr), f);      /* read the header structure */

	if (hdr.magic != FONTMAGIC)		/* check magic number */
	{
		pack_fclose(f);
		return NULL;
	}

	numchar = hdr.maxchar - hdr.minchar + 1;

	if (!hdr.isfixed)                    /* proportional font */
	{
		wtable = malloc(sizeof(unsigned short) * numchar);
		pack_fread(wtable, sizeof(unsigned short) * numchar, f);
	}

	bmp = load_grx_bmps(f, &hdr, numchar, wtable);
	if (!bmp)
		goto get_out;

	if (pack_ferror(f))
		goto get_out;

	font = malloc(sizeof(FONT));
	font->height = -1;
	font->dat.dat_prop = font_prop = malloc(sizeof(FONT_PROP));
	font_prop->render = NULL;

	start = 32 - hdr.minchar;
	width = hdr.width;

	for (c = 0; c  <FONT_SIZE; c++)
	{
		c2 = c+start;

		if ((c2 >= 0) && (c2 < numchar))
		{
			if (!hdr.isfixed)
				width = wtable[c2];

			font_prop->dat[c] = create_bitmap_ex(8, width, hdr.height);
			memcpy(font_prop->dat[c]->dat, bmp[c2], width * hdr.height);
		}
		else
		{
			font_prop->dat[c] = create_bitmap_ex(8, 8, hdr.height);
			clear(font_prop->dat[c]);
		}
	}

	get_out:

	pack_fclose(f);

	if (wtable)
	free(wtable);

	if (bmp)
	{
		for (c = 0; c < numchar; c++)
			free(bmp[c]);

		free(bmp);
	}

	return font;
}


/*
 * Initialize the terminal windows
 */
static bool init_windows(void)
{
	int i, num_windows;

	term_data *td;

	char section[80];

	char filename[1024];

	char buf[128];

	/* Section name */
	sprintf(section, "Mode-%d", resolution);

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
		sprintf(section, "Term-%d-%d", resolution, i);

		/* Term number */
		td->number = i;

		/* Coordinates of left top corner */
		td->x = get_config_int(section, "x", 0);
		td->y = get_config_int(section, "y", 0);

		/* Rows and cols of term */
		td->rows = get_config_int(section, "rows", 24);
		td->cols = get_config_int(section, "cols", 80);

		/* Tile size */
		td->tile_wid = get_config_int(section, "tile_wid", 8);
		td->tile_hgt = get_config_int(section, "tile_hgt", 13);

		/* Font size */
		td->font_wid = get_config_int(section, "tile_wid", 8);
		td->font_hgt = get_config_int(section, "tile_hgt", 13);

		/* Get font filename */
		strcpy(buf, get_config_string(section, "font_file", "xm8x13.fnt"));

		/* Build the name of the font file */
		path_build(filename, 1024, xtra_font_dir, buf);

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

		/* Link the term */
		term_data_link(td);
		angband_term[i] = &td->t;
	}

	/* Success */
	return 0;
}


#ifdef USE_BACKGROUND

/*
 * Initialize the window backgrounds
 */
static void init_background(void)
{
	int i;

	char filename[1024];

	char buf[128];

	PALLETE background_pallete;

	/* Get the backgrounds */
	for (i = 0; i < 16; i++)
	{
		/* Get background filename */
		strcpy(buf, get_config_string("Background", format("Background-%d", i), ""));

		/* Build the filename for the background-bitmap */
		path_build(filename, 1024, xtra_graf_dir, buf);

		/* Try to open the bitmap file */
		background[i] = load_bitmap(filename, background_pallete);
	}

#ifndef USE_SPECIAL_BACKGROUND
	/*
	 * Set the palette for the background
	 */
	if (background[0])
	{
		set_palette_range(background_pallete, 0, COLOR_OFFSET - 1, 0);
	}
#endif /* USE_SPECIAL_BACKGROUND */
}

#endif /* USE_BACKGROUND */


#ifdef USE_GRAPHICS

/*
 * Initialize graphics
 */
static bool init_graphics(void)
{
	char filename[1024];
	char section[80];
	char name_tiles[128];

	/* Large bitmap for the tiles */
	BITMAP *tiles = NULL;
	PALLETE tiles_pallete;

	/* Size of each bitmap tile */
	int bitmap_wid;
	int bitmap_hgt;

	int num_windows;

	if (!graphics_initialized)
	{
		/* Section name */
		sprintf(section, "Mode-%d", resolution);

		/* Get bitmap tile size */
		bitmap_wid = get_config_int(section, "bitmap_wid", 8);
		bitmap_hgt = get_config_int(section, "bitmap_hgt", 8);

		/* Get bitmap filename */
		strcpy(name_tiles, get_config_string(section, "bitmap_file", "8x8.bmp"));

		/* Get number of windows */
		num_windows = get_config_int(section, "num_windows", 1);

		/* Build the name of the bitmap file */
		path_build(filename, 1024, xtra_graf_dir, name_tiles);

		/* Open the bitmap file */
		if ((tiles = load_bitmap(filename, tiles_pallete)) != NULL)
		{
			int i;

			/*
			 * Set the graphics mode to "new" if Adam Bolt's
			 * new 16x16 tiles are used.
			 */
			ANGBAND_GRAF = get_config_string(section, "graf-mode", "old");

			/* Use transparent blits */
			if (streq(ANGBAND_GRAF, "new"))
				use_transparency = TRUE;

			/* Select the bitmap pallete */
			set_palette_range(tiles_pallete, 0, COLOR_OFFSET - 1, 0);

			/* Prepare the graphics */
			for (i = 0; i < num_windows; i++)
			{
				term_data *td;

				int col, row;
				int cols, rows;
				int width, height;
				int src_x, src_y;
				int tgt_x, tgt_y;

				td = &data[i];

				cols = tiles->w / bitmap_wid;
				rows = tiles->h / bitmap_hgt;

				width = td->tile_wid * cols;
				height = td->tile_hgt * rows;

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

			/* Free the old tiles bitmap */
			if (tiles) destroy_bitmap(tiles);

			graphics_initialized = TRUE;

			/* Success */
			return (TRUE);
		}

		/* Failure */
		return (FALSE);
	}

	/* Success */
	return (TRUE);
}

#endif /* USE_GRAPHICS */

#ifdef USE_SOUND

/*
 * Initialize sound
 * We try to get a list of the available sound-files from "lib/xtra/sound/sound.cfg"
 * and then preload the samples. Every Angband-sound-event can have several samples
 * assigned. Angband will randomly select which is played. This makes it easy to
 * create "sound-packs", just copy wav-files into the "lib/xtra/sound/" folder and
 * add the filenames to "sound.cfg" in the same folder.
 */
static bool init_sound(void)
{
	int i, j, done;

	char section[128];
	char filename[1024];
	char **argv;

	struct ffblk f;

	if (sound_initialized) return (TRUE);

	reserve_voices(16, -1);

	/* Initialize Allegro sound */
	if (!install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL))
	{
#ifdef USE_MOD_FILES
		/*
		 * Try to enable support for MOD-, and S3M-files
		 * The parameter for install_mod() is the number
		 * of channels reserved for the MOD/S3M-file.
		 */
		if (install_mod(8) > 0) mod_file_initialized = TRUE;
#endif /* USE_MOD_FILES */

		/* Access the new sample */
		path_build(filename, 1024, xtra_sound_dir, "sound.cfg");

		/* Read config info from "lib/xtra/sound/sound.cfg" */
		override_config_file(filename);

		/* Sound section */
		strcpy(section, "Sound");

		/* Prepare the sounds */
		for (i = 1; i < SOUND_MAX; i++)
		{
			/* Get the sample names */
			argv = get_config_argv(section, angband_sound_name[i], &sample_count[i]);

			/* Limit the number of samples */
			if (sample_count[i] > SAMPLE_MAX) sample_count[i] = SAMPLE_MAX;

			for (j = 0; j < sample_count[i]; j++)
			{
				/* Access the new sample */
				path_build(filename, 1024, xtra_sound_dir, argv[j]);

				/* Load the sample */
				samples[i][j] = load_sample(filename);
			}
		}

		/*
		 * Get a list of music files
		 */
#ifdef USE_MOD_FILES
		if (mod_file_initialized)
		{
			done = findfirst(format("%s/*.*", xtra_music_dir), &f, FA_ARCH|FA_RDONLY);
		}
		else
#endif /* USE_MOD_FILES */
		done = findfirst(format("%s/*.mid", xtra_music_dir), &f, FA_ARCH|FA_RDONLY);


		while (!done && (song_number <= MAX_SONGS))
		{
			/* Add music files */
			{
				strcpy(music_files[song_number], f.ff_name);
				song_number++;
			}

			done = findnext(&f);
		}

		/* Use "angdos.cfg" */
		override_config_file("angdos.cfg");

		/* Sound section */
		strcpy(section, "Sound");

		/* Get the volume setting */
		digi_volume = get_config_int(section, "digi_volume", 255);
		midi_volume = get_config_int(section, "midi_volume", 255);

		/* Set the volume */
		set_volume(digi_volume, midi_volume);

		/* Success */
		return (TRUE);
	}

	/* Init failed */
	return (FALSE);
}


/*
 * Make a sound
 */
static errr Term_xtra_dos_sound(int v)
{
	int n;

	/* Sound disabled */
	if (!use_sound) return (1);

	/* Illegal sound */
	if ((v < 0) || (v >= SOUND_MAX)) return (1);

	/* Get a random sample from the available ones */
	n = rand_int(sample_count[v]);

	/* Play the sound, catch errors */
	if (samples[v][n])
	{
		return (play_sample(samples[v][n], 255, 128, 1000, 0) == 0);
	}

	/* Oops */
	return (1);
}


/*
 * Play a song-file
 */
static void play_song(void)
{
	char filename[256];

	/* Clear the old song */
	if (midi_song) destroy_midi(midi_song);
	midi_song = NULL;

#ifdef USE_MOD_FILES
	if (mod_file_initialized)
	{
		stop_mod();
		destroy_mod(mod_song);
	}
#endif /* USE_MOD_FILES */

	/* Access the new song */
	path_build(filename, 1024, xtra_music_dir, music_files[current_song - 1]);

	/* Load and play the new song */
	midi_song = load_midi(filename);

	if (midi_song)
	{
		play_midi(midi_song, 0);
	}
#ifdef USE_MOD_FILES
	else if (mod_file_initialized)
	{
		mod_song = load_mod(filename);

		if (mod_song) play_mod(mod_song, FALSE);
	}
#endif /* USE_MOD_FILES */
}

#endif /* USE_SOUND */


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
	term_data *td;

	char section[80];

	int screen_wid;
	int screen_hgt;

	/* Initialize the Allegro library (never fails) */
	(void)allegro_init();

	/* Install timer support for music and sound */
	install_timer();

	/* Read config info from filename */
	set_config_file("angdos.cfg");

	/* Main section */
	strcpy(section, "Angband");

	/* Get screen size */
	resolution = get_config_int(section, "Resolution", 1);

	/* Section name */
	sprintf(section, "Mode-%d", resolution);

	/* Get the screen dimensions */
	screen_wid = get_config_int(section, "screen_wid", 640);
	screen_hgt = get_config_int(section, "screen_hgt", 480);

	/* Set the color depth */
	set_color_depth(8);

	/* Auto-detect, and instantiate, the appropriate graphics mode */
	if ((set_gfx_mode(GFX_AUTODETECT, screen_wid, screen_hgt, 0, 0)) < 0)
	{
		/*
		 * Requested graphics mode is not available
		 * We retry with the basic 640x480 mode
		 */
		resolution = 1;

		if ((set_gfx_mode(GFX_AUTODETECT, 640, 480, 0, 0)) < 0)
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
	}

	/* Hook in "z-util.c" hook */
	quit_aux = dos_quit_hook;

	/* Build the "graf" path */
	path_build(xtra_graf_dir, 1024, ANGBAND_DIR_XTRA, "graf");

	/* Build the "font" path */
	path_build(xtra_font_dir, 1024, ANGBAND_DIR_XTRA, "font");

	/* Build the "sound" path */
	path_build(xtra_sound_dir, 1024, ANGBAND_DIR_XTRA, "sound");

	/* Build the "music" path */
	path_build(xtra_music_dir, 1024, ANGBAND_DIR_XTRA, "music");

	/* Initialize the windows */
	init_windows();

#ifdef USE_SOUND

	/* Look for the sound preferences in "angdos.cfg" */
	if (!arg_sound)
	{
		arg_sound = get_config_int("Angband", "Sound", TRUE);
	}

#endif /* USE_SOUND */

#ifdef USE_GRAPHICS

	/* Look for the graphic preferences in "angdos.cfg" */
	if (!arg_graphics)
	{
		arg_graphics = get_config_int("Angband", "Graphics", GRAPHICS_ORIGINAL);
	}

#endif /* USE_GRAPHICS */

	/* Initialize the "complex" RNG for the midi-shuffle function */
	Rand_quick = FALSE;
	Rand_state_init(time(NULL));

	/* Set the Angband colors/graphics/sound mode */
	Term_xtra_dos_react();

#ifdef USE_BACKGROUND

	/* Initialize the background graphics */
	init_background();

#endif /* USE_BACKGROUND */

	/* Clear the screen */
	clear_to_color(screen, COLOR_OFFSET + TERM_DARK);

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

	/* Place the cursor */
	Term_curs_dos(0, 0);

#ifdef USE_BACKGROUND

	/* Use transparent text */
	text_mode(-1);

#endif /* USE_BACKGROUND */

	/* Success */
	return 0;
}

#endif /* USE_DOS */

