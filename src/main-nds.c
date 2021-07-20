/**
 * \file main-nds.c
 * \brief Main file for playing on the Nintendo DS
 *
 * Copyright (c) 2010 Nick McConnell
 *
 * Many of the routines are based on (or lifted directly from) brettk's
 * excellent NethackDS: http://frodo.dyn.gno.org/~brettk/NetHackDS
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include <fat.h>
#include <nds.h>

#include "angband.h"
#include "buildid.h"
#include "init.h"
#include "main.h"
#include "savefile.h"
#include "ui-prefs.h"
#include "ui-term.h"

/* DS includes */
#include "nds/ds_font_3x8.h"
#include "nds/ds_main.h"

#define NDS_BUTTON_FILE "buttons.dat"

#define NDS_MAPPABLE_MASK (KEY_A | KEY_B | KEY_X | KEY_Y | KEY_START | KEY_SELECT)
#define NDS_MODIFIER_MASK (KEY_L | KEY_R)
#define NDS_BUTTON_MASK (NDS_MAPPABLE_MASK | NDS_MODIFIER_MASK)
#define NDS_NUM_MAPPABLE 6 /* A, B, X, Y, Select, Start */
#define NDS_NUM_MODIFIER 2 /* R, L */
#define NDS_CMD_LENGTH 16  /* max. 15 keys/button + null terminator */

/* [mappable]*2^[mods] things to map commands to, [cmd_length] chars per command */
byte nds_btn_cmds[NDS_NUM_MAPPABLE << NDS_NUM_MODIFIER][NDS_CMD_LENGTH];

/* make sure there's something there to start with */
byte btn_defaults[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                       'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
                       'q', 'r', 's', 't', 'u', 'v', 'w', 'z'};

const s16 mappables[] = {KEY_A, KEY_B, KEY_X, KEY_Y, KEY_SELECT, KEY_START};
const s16 modifiers[] = {KEY_L, KEY_R};
s16 nds_buttons_to_btnid(u16 kd, u16 kh)
{
	if (!(kd & NDS_MAPPABLE_MASK))
		return -1;
	u16 i, mods = 0;
	for (i = 0; i < NDS_NUM_MODIFIER; i++) {
		if (kh & modifiers[i])
			mods |= (1 << i);
	}
	for (i = 0; i < NDS_NUM_MAPPABLE; i++) {
		if (kd & mappables[i])
			return i + NDS_NUM_MAPPABLE * (mods);
	}
	return -1;
}

const nds_kbd_key row0[] = {{16, (u16)'`'}, {16, (u16)'1'},  {16, (u16)'2'},
                            {16, (u16)'3'}, {16, (u16)'4'},  {16, (u16)'5'},
                            {16, (u16)'6'}, {16, (u16)'7'},  {16, (u16)'8'},
                            {16, (u16)'9'}, {16, (u16)'0'},  {16, (u16)'-'},
                            {16, (u16)'='}, {32, (u16)'\b'}, {0, 0}};
const nds_kbd_key row1[] = {{24, (u16)'\t'}, {16, (u16)'q'},  {16, (u16)'w'},
                            {16, (u16)'e'},  {16, (u16)'r'},  {16, (u16)'t'},
                            {16, (u16)'y'},  {16, (u16)'u'},  {16, (u16)'i'},
                            {16, (u16)'o'},  {16, (u16)'p'},  {16, (u16)'['},
                            {16, (u16)']'},  {24, (u16)'\\'}, {0, 0}};
const nds_kbd_key row2[] = {{32, K_CAPS},    {16, (u16)'a'}, {16, (u16)'s'},
                            {16, (u16)'d'},  {16, (u16)'f'}, {16, (u16)'g'},
                            {16, (u16)'h'},  {16, (u16)'j'}, {16, (u16)'k'},
                            {16, (u16)'l'},  {16, (u16)';'}, {16, (u16)'\''},
                            {32, (u16)'\n'}, {0, 0}};
const nds_kbd_key row3[] = {{40, K_SHIFT},  {16, (u16)'z'}, {16, (u16)'x'},
                            {16, (u16)'c'}, {16, (u16)'v'}, {16, (u16)'b'},
                            {16, (u16)'n'}, {16, (u16)'m'}, {16, (u16)','},
                            {16, (u16)'.'}, {16, (u16)'/'}, {40, K_SHIFT},
                            {0, 0}};
const nds_kbd_key row4[] = {{32, K_CTRL}, {24, K_ALT},  {128, (u16)' '},
                            {24, K_ALT},  {32, K_CTRL}, {0, 0}};
char shifts[] = "`~1!2@3#4$5%6^7&8*9(0)-_=+[{]}\\|;:\'\",<.>/?";
const nds_kbd_key *kbdrows[] = {row0, row1, row2, row3, row4};

/*
 * Extra data to associate with each "window"
 *
 * Each "window" is represented by a "term_data" structure, which
 * contains a "term" structure, which contains a pointer (t->data)
 * back to the term_data structure.
 */

typedef struct term_data term_data;

struct term_data {
	term t;
};

/*
 * Number of "term_data" structures to support XXX XXX XXX
 *
 * You MUST support at least one "term_data" structure, and the
 * game will currently use up to eight "term_data" structures if
 * they are available.
 *
 * If only one "term_data" structure is supported, then a lot of
 * the things that would normally go into a "term_data" structure
 * could be made into global variables instead.
 */
#define MAX_TERM_DATA 1

/*
 * An array of "term_data" structures, one for each "sub-window"
 */
static term_data data[MAX_TERM_DATA];

/*
 * Colour data
 */

u16b color_data[] = {
    RGB15(0, 0, 0),    /* COLOUR_DARK */
    RGB15(31, 31, 31), /* COLOUR_WHITE */
    RGB15(15, 15, 15), /* COLOUR_SLATE */
    RGB15(31, 15, 0),  /* COLOUR_ORANGE */
    RGB15(23, 0, 0),   /* COLOUR_RED */
    RGB15(0, 15, 9),   /* COLOUR_GREEN */
    RGB15(0, 0, 31),   /* COLOUR_BLUE */
    RGB15(15, 9, 0),   /* COLOUR_UMBER */
    RGB15(9, 9, 9),    /* COLOUR_L_DARK */
    RGB15(23, 23, 23), /* COLOUR_L_WHITE */
    RGB15(31, 0, 31),  /* COLOUR_VIOLET */
    RGB15(31, 31, 0),  /* COLOUR_YELLOW */
    RGB15(31, 0, 0),   /* COLOUR_L_RED */
    RGB15(0, 31, 0),   /* COLOUR_L_GREEN */
    RGB15(0, 31, 31),  /* COLOUR_L_BLUE */
    RGB15(23, 15, 9)   /* COLOUR_L_UMBER */
};

/*** Function hooks needed by "Term" ***/

/*
 * Init a new "term"
 *
 * This function should do whatever is necessary to prepare a new "term"
 * for use by the "term.c" package.  This may include clearing the window,
 * preparing the cursor, setting the font/colors, etc.  Usually, this
 * function does nothing, and the "init_xxx()" function does it all.
 */
static void Term_init_nds(term *t)
{
	term_data *td = (term_data *)(t->data);

	/* XXX XXX XXX */
}

/*
 * Nuke an old "term"
 *
 * This function is called when an old "term" is no longer needed.  It should
 * do whatever is needed to clean up before the program exits, such as wiping
 * the screen, restoring the cursor, fixing the font, etc.  Often this function
 * does nothing and lets the operating system clean up when the program quits.
 */
static void Term_nuke_nds(term *t)
{
	term_data *td = (term_data *)(t->data);

	/* XXX XXX XXX */
}

/*
 * Find the square a particular pixel is part of.
 */
static void pixel_to_square(int *const x, int *const y, const int ox,
                            const int oy)
{
	(*x) = ox / NDS_FONT_WIDTH;
	(*y) = oy / NDS_FONT_HEIGHT;
}

/*
 * Handle a touch on the touch screen.
 */
static void handle_touch(int x, int y, int button, bool press)
{
	/* The co-ordinates are only used in Angband format. */
	pixel_to_square(&x, &y, x, y);

	if (press)
		Term_mousepress(x, y, button);
}

/*
 * Touchscreen keyboard handling
 */

static bool shift = false, ctrl = false, alt = false, caps = false;

u16b kbd_mod_code(u16 ret)
{
	if (ret & K_MODIFIER)
		return ret;
	if (caps && !shift) {
		if (ret >= 'a' && ret <= 'z')
			ret -= 0x20;
	}
	if (shift) {
		char *temp;
		if (!caps && ret >= 'a' && ret <= 'z')
			ret -= 0x20;
		if ((temp = strchr(shifts, ret)) != NULL)
			ret = *(temp + 1);
	}
	if (alt) {
		ret |= 0x80;
	}
	if (ctrl /* && ret >= 'a' && ret < 'a'+32*/) {
		ret = ret & 0x1f;
	}
	return ret;
}

void kbd_set_color_from_pos(u16b r, u16b k, byte color)
{
	u16b ii, xx = 0, jj;
	u16b *map[] = {(u16b *)(BG_MAP_RAM_SUB(8) + 3 * 32 * 2),
	               (u16b *)(BG_MAP_RAM_SUB(9) + 3 * 32 * 2),
	               (u16b *)(BG_MAP_RAM_SUB(10) + 3 * 32 * 2),
	               (u16b *)(BG_MAP_RAM_SUB(11) + 3 * 32 * 2)};
	for (ii = 0; ii < k; ii++) {
		xx += kbdrows[r][ii].width >> 3;
	}
	for (ii = 0; ii < (kbdrows[r][k].width >> 3); ii++) {
		for (jj = 0; jj < 4; jj++) {
			map[jj][(10 + r * 2) * 32 + ii + xx + 1] =
			    (map[jj][(10 + r * 2) * 32 + ii + xx + 1] &
			     0x0FFF) |
			    (color << 12);
			map[jj][(10 + r * 2 + 1) * 32 + ii + xx + 1] =
			    (map[jj][(10 + r * 2 + 1) * 32 + ii + xx + 1] &
			     0x0FFF) |
			    (color << 12);
		}
	}
}

void kbd_set_color_from_code(u16b code, byte color)
{
	u16b r, k;
	for (r = 0; r < 5; r++) {
		for (k = 0; kbdrows[r][k].width != 0; k++) {
			if (kbd_mod_code(kbdrows[r][k].code) == code) {
				kbd_set_color_from_pos(r, k, color);
			}
			/* 
			 * do not break!! there may be >1 key with this code
			 * (modifier keys)
			 */
		}
	}
}

void kbd_set_map()
{
	REG_BG0CNT_SUB = BG_TILE_BASE(0) |
	                 BG_MAP_BASE(8 + (caps | (shift << 1))) |
	                 BG_PRIORITY(0) | BG_COLOR_16;
}

u16b kbd_xy2key(byte x, byte y)
{
	/* on arrow-pad */
	if (x >= 104 && x < 152 && y >= 24 && y < 72) {
		byte kx = (x - 104) / 16, ky = (y - 24) / 16;
		return (kx + (2 - ky) * 3 + 1 +
		        '0') /* | (shift ? K_SHIFTED_MOVE : 0)*/;
	}

	if (y >= 80 && y < 96) {
		if (x >= 8 && x < 24)
			return '\033';

		/* F-key */
		if (x >= 40 && x < 248) {
			x -= 40;
			y = x / 72;  /* which section */
			x -= y * 72; /* offset in section */
			if (x < 64) {
				/* section*4 + offset/16 + 1 */
				return K_F(y * 4 + (x >> 4) + 1); 
			} else {
				return 0;
			}
		}
	}

	s16b ox = x - 8, oy = y - 104;
	if (ox < 0 || ox >= 240)
		return 0;
	if (oy < 0 || oy >= 80)
		return 0;
	u16b row = oy / 16;
	int i;
	for (i = 0; ox > 0; ox -= kbdrows[row][i++].width)
		;
	u16b ret = kbdrows[row][i - 1].code;
	return kbd_mod_code(ret);
}

void kbd_dotoggle(bool *flag, int how)
{
	switch (how) {
	case 0:
		*flag = false;
		return;
	case 1:
		*flag = true;
		return;
	default:
	case -1:
		*flag = !*flag;
		return;
	}
}

/*
 * which: K_SHIFT, K_CTRL, K_ALT, K_MODIFIER=all keys
 * how: -1 = toggle, 0 = off, 1 = on
 */
void kbd_togglemod(int which, int how)
{
	/* boolean old_shift = shift, old_ctrl = ctrl, old_alt = alt, old_caps = caps; */
	switch (which) {
	case K_CTRL:
		kbd_dotoggle(&ctrl, how);
		break;
	case K_SHIFT:
		kbd_dotoggle(&shift, how);
		break;
	case K_ALT:
		kbd_dotoggle(&alt, how);
		break;
	case K_CAPS:
		kbd_dotoggle(&caps, how);
		break;
	case K_MODIFIER:
		kbd_dotoggle(&ctrl, how);
		kbd_dotoggle(&shift, how);
		kbd_dotoggle(&alt, how);
		/*
		 * NOT caps!!  This is called to un-set shift, ctrl, and alt after
		 * a key is pressed.  Unsetting caps here would cause it to be the
		 * same as shift.
		 */
		break;
	}

	/* if (old_shift != shift) */
	kbd_set_color_from_code(K_SHIFT, shift);

	/* if (old_ctrl != ctrl) */
	kbd_set_color_from_code(K_CTRL, ctrl);

	/* if (old_alt != alt) */
	kbd_set_color_from_code(K_ALT, alt);

	/* if (old_caps != caps) */
	kbd_set_color_from_code(K_CAPS, caps);

	kbd_set_map();
}

/*
 * clear this to prevent alt-b, f5, and f6 from having their special effects
 * it's cleared during getlin, yn_function, etc
 */
byte process_special_keystrokes = 1;

/* 
 * run this every frame
 * returns a key code if one has been typed, else returns 0
 * assumes scankeys() was already called this frame (in real vblank handler)
 */
byte kbd_vblank()
{
	/* frames the stylus has been held down for */
	static u16b touched = 0;
	/* coordinates from each frame, the median is used to get the keycode */
	static s16b xarr[3], yarr[3];
	/* the keycode of the last key pressed, so it can be un-highlighted */
	static u16b last_code;
	/* the keycode of the currently pressed key, is usu. returned */
	u16b keycode;
	/* current input data */
	touchPosition touch;

	touchRead(&touch);

	/* if screen is being touched... */
	if (keysHeld() & KEY_TOUCH) {
		if (touched < 3) { /* if counter < 3... */
			touched++; /* add to counter */
			xarr[touched - 1] =
			    touch.px; /* add this to the array for */
			yarr[touched - 1] = touch.py; /* finding the median */
		}
	} else {             /* not being touched */
		touched = 0; /* so reset the counter for next time */
	}

	/* if the stylus was released */
	if (keysUp() & KEY_TOUCH) {
		/* if last_code is set and it wasn't a modifier */
		if (last_code && !(last_code & K_MODIFIER)) {
			/* clear the hiliting on this key */
			kbd_set_color_from_code(last_code, 0);
			/* and also clear all modifiers (except caps)    */
			kbd_togglemod(K_MODIFIER, 0);
		}
		last_code = 0;
	}

	/* if the screen has been touched for 3 frames... */
	if (touched == 3) {
		touched++; /* do not return the keycode again */
		/* also, not setting to zero prevents the keysHeld() thing */
		/*  from starting the process over and getting 3 more samples */

		u16b i, tmp, the_x = 0, the_y = 0;

		/* x/yarr now contains 3 values from each of the 3 frames */
		/* take the median of each array and put into the_x/y */

		/* sort the array */
		/* bubble sort, ugh */
		for (i = 1; i < 3; i++) {
			if (xarr[i] < xarr[i - 1]) {
				tmp = xarr[i];
				xarr[i] = xarr[i - 1];
				xarr[i - 1] = tmp;
			}
			if (yarr[i] < yarr[i - 1]) {
				tmp = yarr[i];
				yarr[i] = yarr[i - 1];
				yarr[i - 1] = tmp;
			}
		}

		/* get the middle value (median) */
		/* if it's -1, take the top value */
		if (xarr[1] == -1)
			the_x = xarr[2];
		else
			the_x = xarr[1];
		if (yarr[1] == -1)
			the_y = yarr[2];
		else
			the_y = yarr[1];

		/* get the keycode that corresponds to this key */
		u16b keycode = kbd_xy2key(the_x, the_y);

		/* if it's not a modifier, highlight it */
		if (keycode && !(keycode & K_MODIFIER))
			kbd_set_color_from_code(keycode, 1);
		/* set last_code so it can be un-highlighted later */
		last_code = keycode;

		/*/* check for special keystrokes: alt-b, f5, f6 */
		if (process_special_keystrokes) {
			/* alt-b: assign button macro */
			if (keycode == ('b' | 0x80)) {
				/* clear hiliting */
				kbd_set_color_from_code(keycode, 0);
				kbd_togglemod(K_MODIFIER, 0);
				// nds_assign_button();
				keycode = last_code =
				    0; /* don't let nethack process it */
			}

			if (keycode & K_F(0)) { /* its an f-key */
				kbd_togglemod(K_MODIFIER, 0);
			}
		}

		/* if it's a modifier, toggle it */
		if (keycode & K_MODIFIER)
			kbd_togglemod(keycode, -1);
		else if ((keycode & 0x7F) !=
		         0) { /* it's an actual keystroke, return it */
			return (keycode & 0xFF);
		}
	}

	return 0;
}

void nds_check_buttons(u16b kd, u16b kh)
{
	s16b btn = nds_buttons_to_btnid(kd, kh);
	if (btn == -1)
		return;
	byte *cmd = &nds_btn_cmds[btn][0];
	while (*cmd != 0) {
		put_key_event(*(cmd++));
	}
}

/*
 * All event handling
 */
u16b *ebuf = (u16b *)(&BG_GFX[256 * 192]);
/* store the queue just past mainscreen display data */
u16b ebuf_read = 0, ebuf_write = 0;
byte nds_updated = 0; /* windows that have been updated and should be redrawn */

bool has_event()
{
	return ((ebuf[ebuf_read] & EVENT_SET) || (ebuf_read < ebuf_write));
	/* read < write should never happen without EVENT_SET, but */
	/* just in case... */
}

u16b get_event()
{
	if (!has_event())
		return 0;
	u16b r = ebuf[ebuf_read];
	ebuf[ebuf_read] = 0;
	ebuf_read++;
	if (ebuf_read > ebuf_write) {
		ebuf_write++;
		if (ebuf_write >= MAX_EBUF)
			ebuf_write = 0;
	}
	if (ebuf_read >= MAX_EBUF)
		ebuf_read = 0;
	return r;
}

void put_key_event(byte c)
{
	ebuf[ebuf_write++] = EVENT_SET | (u16)c;
	if (ebuf_write >= MAX_EBUF)
		ebuf_write = 0;
}

void put_mouse_event(byte x, byte y)
{
	ebuf[ebuf_write++] =
	    EVENT_SET | MEVENT_FLAG | (u16b)x | (((u16b)y) << 7);
	if (ebuf_write >= MAX_EBUF)
		ebuf_write = 0;
}

void do_vblank()
{
	swiWaitForVBlank();

	/* --------------------------- */
	/*  Handle the arrow buttons */
	scanKeys();
	u32b kd = keysDown();
	u32b kh = keysHeld();
	/* order of keys: Right, Left, Up, Down */
	/* map keys to dirs, depends on order of keys in nds/input.h */
	/*  and order of directions in ndir & sdir in decl.c */
	/*const s8 k2d[] = {	// indexes into ndir/sdir, 10 = end of string
	 * = '\0' */
	/* 10, 4, 0, 10, 2, 3, 1, 10, 6, 5, 7	// no working combinations >=
	 * 11 */
	/*}; */
	const byte k2d[] = {'6', '4', '8', '2', '3', '7', '9', '1'};
	/* only do stuff if a key was pressed last frame */
	if (kd & (KEY_RIGHT | KEY_LEFT | KEY_UP | KEY_DOWN)) {
		u16b dirs_down = 0;
		int i;
		if (kh & KEY_LEFT)
			dirs_down++;
		if (kh & KEY_RIGHT)
			dirs_down++;
		if (kh & KEY_UP)
			dirs_down++;
		if (kh & KEY_DOWN)
			dirs_down++;
		if (dirs_down == 1 && !(kh & (KEY_R | KEY_L))) {
			for (i = 0; i < 4; i++)
				if (kh & (1 << (i + 4)))
					put_key_event(k2d[i]);
		} else if (dirs_down == 2 && (kh & (KEY_R | KEY_L))) {
			for (i = 0; i < 4; i++)
				if (kh & (1 << (i + 4)))
					put_key_event(k2d[i + 4]);
		}
	}

	/* --------------------------- */
	/*  Check for button macros */
	nds_check_buttons(kd, kh);

	/* --------------------------- */
	/*  Check for typing on the touchscreen kbd */
	byte keycode = kbd_vblank();
	if ((keycode & 0x7F) != 0) { /* it's an actual keystroke, return it */
		put_key_event(keycode & 0xFF);
		/*Term_keypress(keycode & 0xFF); */
	}
}

/*END JUST MOVED */

/*
 * An event handler XXX XXX XXX
 *
 * You may need an event handler, which can be used by both
 * by the "TERM_XTRA_BORED" and "TERM_XTRA_EVENT" entries in
 * the "Term_xtra_xxx()" function, and also to wait for the
 * user to perform whatever user-interface operation is needed
 * to request the start of a new game or the loading of an old
 * game, both of which should launch the "play_game()" function.
 */
static errr CheckEvents(bool wait)
{
	u16b e = 0;

	do_vblank();

	if (!wait && !has_event())
		return (1);

	while (!e) {
		e = get_event();

		do_vblank();
	}

	/* Mouse */
	if (IS_MEVENT(e))
		handle_touch(EVENT_X(e) + 1, EVENT_Y(e), 1, true);

	/* Undefined */
	else if ((EVENT_C(e) & 0x7F) == 0)
		return (1);

	/* Key */
	else
		Term_keypress(EVENT_C(e), 0);

	return (0);
}

/*
 * Do a "special thing" to the current "term"
 *
 * This function must react to a large number of possible arguments, each
 * corresponding to a different "action request" by the "ui-term.c" package,
 * or by the application itself.
 *
 * The "action type" is specified by the first argument, which must be a
 * constant of the form "TERM_XTRA_*" as given in "term.h", and the second
 * argument specifies the "information" for that argument, if any, and will
 * vary according to the first argument.
 *
 * In general, this function should return zero if the action is successfully
 * handled, and non-zero if the action is unknown or incorrectly handled.
 */
static errr Term_xtra_nds(int n, int v)
{
	term_data *td = (term_data *)(Term->data);

	/* Analyze */
	switch (n) {
	case TERM_XTRA_EVENT: {
		/*
		 * Process some pending events
		 */
		return (CheckEvents(v));
	}

	case TERM_XTRA_FLUSH: {
		/*
		 * Flush all pending events
		 */
		while (!CheckEvents(false))
			;

		return (0);
	}

	case TERM_XTRA_CLEAR: {
		/*
		 * Clear the entire window
		 */
		int x, y;
		u32b vram_offset;
		u16b *fb = BG_GFX;

		for (y = 0; y < 24; y++) {
			for (x = 0; x < 80; x++) {
				vram_offset = (y & 0x1F) * 8 * 256 + x * 3;

				byte xx, yy;
				for (yy = 0; yy < 8; yy++)
					for (xx = 0; xx < 3; xx++)
						fb[yy * 256 + xx +
						   vram_offset] = 0;
			}
		}

		return (0);
	}

	case TERM_XTRA_SHAPE: {
		/*
		 * Set the cursor visibility XXX XXX XXX
		 *
		 * This action should change the visibility of the cursor,
		 * if possible, to the requested value (0=off, 1=on)
		 *
		 * This action is optional, but can improve both the
		 * efficiency (and attractiveness) of the program.
		 */

		return (0);
	}

	case TERM_XTRA_FROSH: {
		return (0);
	}

	case TERM_XTRA_FRESH: {
		return (0);
	}

	case TERM_XTRA_NOISE: {
		/*
		 * Make a noise XXX XXX XXX
		 *
		 * This action should produce a "beep" noise.
		 *
		 * This action is optional, but convenient.
		 */

		return (0);
	}

	case TERM_XTRA_BORED: {
		/*
		 * Handle random events when bored
		 */
		return (CheckEvents(0));
	}

	case TERM_XTRA_REACT: {
		/*
		 * React to global changes XXX XXX XXX
		 *
		 * For example, this action can be used to react to
		 * changes in the global "color_table[256][4]" array.
		 *
		 * This action is optional, but can be very useful for
		 * handling "color changes" and the "arg_sound" and/or
		 * "arg_graphics" options.
		 */

		return (0);
	}

	case TERM_XTRA_ALIVE: {
		/*
		 * Change the "hard" level XXX XXX XXX
		 *
		 * This action is used if the program changes "aliveness"
		 * by being either "suspended" (v=0) or "resumed" (v=1)
		 * This action is optional, unless the computer uses the
		 * same "physical screen" for multiple programs, in which
		 * case this action should clean up to let other programs
		 * use the screen, or resume from such a cleaned up state.
		 *
		 * This action is currently only used by "main-gcu.c",
		 * on UNIX machines, to allow proper "suspending".
		 */

		return (0);
	}

	case TERM_XTRA_LEVEL: {
		/*
		 * Change the "soft" level XXX XXX XXX
		 *
		 * This action is used when the term window changes "activation"
		 * either by becoming "inactive" (v=0) or "active" (v=1)
		 *
		 * This action can be used to do things like activate the proper
		 * font / drawing mode for the newly active term window.  This
		 * action should NOT change which window has the "focus", which
		 * window is "raised", or anything like that.
		 *
		 * This action is optional if all the other things which depend
		 * on what term is active handle activation themself, or if only
		 * one "term_data" structure is supported by this file.
		 */

		return (0);
	}

	case TERM_XTRA_DELAY: {
		/*
		 * Delay for some milliseconds
		 */
		int i;
		for (i = 0; i < v; i++)
			swiWaitForVBlank();

		return (0);
	}
	}

	/* Unknown or Unhandled action */
	return (1);
}

/*
 * Display the cursor
 */
static errr Term_curs_nds(int x, int y)
{
	u32b vram_offset = y * NDS_FONT_HEIGHT * 256 + x * NDS_FONT_WIDTH;
	byte xx, yy;
	for (xx = 0; xx < NDS_FONT_WIDTH; xx++) {
		BG_GFX[xx + vram_offset] = RGB15(31, 31, 0) | BIT(15);
		BG_GFX[256 * (NDS_FONT_HEIGHT - 1) + xx + vram_offset] =
		    RGB15(31, 31, 0) | BIT(15);
	}
	for (yy = 0; yy < NDS_FONT_HEIGHT; yy++) {
		BG_GFX[yy * 256 + vram_offset] = RGB15(31, 31, 0) | BIT(15);
		BG_GFX[yy * 256 + NDS_FONT_WIDTH - 1 + vram_offset] =
		    RGB15(31, 31, 0) | BIT(15);
	}

	/* Success */
	return (0);
}

void draw_char(byte x, byte y, char c)
{
	u32b vram_offset = (y & 0x1F) * NDS_FONT_HEIGHT * 256 + x * NDS_FONT_WIDTH;

	u16b *fb = BG_GFX;
	if (y & 32) {
		fb = &BG_GFX_SUB[16 * 1024];
	}

	byte xx, yy;
	for (yy = 0; yy < NDS_FONT_HEIGHT; yy++) {
		for (xx = 0; xx < NDS_FONT_WIDTH; xx++) {
			fb[yy * 256 + xx + vram_offset] = nds_font_pixel(c, xx, yy) | BIT(15);
		}
	}
}

void draw_color_char(byte x, byte y, char c, byte clr)
{
	u32b vram_offset = (y & 0x1F) * NDS_FONT_HEIGHT * 256 + x * NDS_FONT_WIDTH;

	u16b *fb = BG_GFX;
	if (y & 32) {
		fb = &BG_GFX_SUB[16 * 1024];
	}

	byte xx, yy;
	u16b fgc = color_data[clr & 0xF];
	for (yy = 0; yy < NDS_FONT_HEIGHT; yy++) {
		for (xx = 0; xx < NDS_FONT_WIDTH; xx++) {
			fb[yy * 256 + xx + vram_offset] = (nds_font_pixel(c, xx, yy) & fgc) | BIT(15);
		}
	}
}

/*
 * Erase some characters
 *
 * This function should erase "n" characters starting at (x,y).
 *
 * You may assume "valid" input if the window is properly sized.
 */
static errr Term_wipe_nds(int x, int y, int n)
{
	term_data *td = (term_data *)(Term->data);

	int i;

	/* Draw a blank */
	for (i = 0; i < n; i++)
		draw_color_char(x + i, y, 0, 0);

	/* Success */
	return (0);
}

/*
 * Draw some text on the screen
 *
 * This function should actually display an array of characters
 * starting at the given location, using the given "attribute",
 * and using the given string of characters, which contains
 * exactly "n" characters and which is NOT null-terminated.
 *
 * You may assume "valid" input if the window is properly sized.
 *
 * You must be sure that the string, when written, erases anything
 * (including any visual cursor) that used to be where the text is
 * drawn.  On many machines this happens automatically, on others,
 * you must first call "Term_wipe_xxx()" to clear the area.
 *
 * In color environments, you should activate the color contained
 * in "color_data[a & 0x0F]", if needed, before drawing anything.
 *
 * You may ignore the "attribute" if you are only supporting a
 * monochrome environment, since this routine is normally never
 * called to display "black" (invisible) text, including the
 * default "spaces", and all other colors should be drawn in
 * the "normal" color in a monochrome environment.
 *
 * Note that if you have changed the "attr_blank" to something
 * which is not black, then this function must be able to draw
 * the resulting "blank" correctly.
 *
 * Note that this function must correctly handle "black" text if
 * the "always_text" flag is set, if this flag is not set, all the
 * "black" text will be handled by the "Term_wipe_xxx()" hook.
 */
static errr Term_text_nds(int x, int y, int n, byte a, const char *cp)
{
	int i;

	/* Do nothing if the string is null */
	if (!cp || !*cp)
		return (-1);

	/* Get the length of the string */
	if ((n > strlen(cp)) || (n < 0))
		n = strlen(cp);

	/* Put the characters directly */
	for (i = 0; i < n, *cp; i++) {
		/* Check it's the right attr */
		if ((x + i < Term->wid) && (Term->scr->a[y][x + i] == a))
			/* Put the char */
			draw_color_char(x + i, y, (*(cp++)), a);
		else
			break;
	}
	/* Success */
	return (0);
}

/*** Internal Functions ***/

/*
 * Instantiate a "term_data" structure
 *
 * This is one way to prepare the "term_data" structures and to
 * "link" the various informational pieces together.
 *
 * This function assumes that every window should be 80x24 in size
 * (the standard size) and should be able to queue 256 characters.
 * Technically, only the "main screen window" needs to queue any
 * characters, but this method is simple.  One way to allow some
 * variation is to add fields to the "term_data" structure listing
 * parameters for that window, initialize them in the "init_xxx()"
 * function, and then use them in the code below.
 *
 * Note that "activation" calls the "Term_init_xxx()" hook for
 * the "term" structure, if needed.
 */
static void term_data_link(int i)
{
	term_data *td = &data[i];

	term *t = &td->t;

	/* Initialize the term */
	term_init(t, 85, 24, 256);

	/* Choose "soft" or "hard" cursor XXX XXX XXX */
	/* A "soft" cursor must be explicitly "drawn" by the program */
	/* while a "hard" cursor has some "physical" existance and is */
	/* moved whenever text is drawn on the screen.  See "term.c". */
	t->soft_cursor = true;

	/* Use "Term_pict()" for all attr/char pairs XXX XXX XXX */
	/* See the "Term_pict_xxx()" function above. */
	/* td->t->always_pict = true; */

	/* Use "Term_pict()" for some attr/char pairs XXX XXX XXX */
	/* See the "Term_pict_xxx()" function above. */
	/* t->higher_pict = true; */

	/* Use "Term_text()" even for "black" text XXX XXX XXX */
	/* See the "Term_text_xxx()" function above. */
	/* t->always_text = true; */

	/* Ignore the "TERM_XTRA_BORED" action XXX XXX XXX */
	/* This may make things slightly more efficient. */
	t->never_bored = true;

	/* Ignore the "TERM_XTRA_FROSH" action XXX XXX XXX */
	/* This may make things slightly more efficient. */
	/* td->t->never_frosh = true; */

	/* Prepare the init/nuke hooks */
	t->init_hook = Term_init_nds;
	t->nuke_hook = Term_nuke_nds;

	/* Prepare the template hooks */
	t->xtra_hook = Term_xtra_nds;
	t->curs_hook = Term_curs_nds;
	t->wipe_hook = Term_wipe_nds;
	t->text_hook = Term_text_nds;

	/* Remember where we came from */
	t->data = (void *)(td);

	/* Activate it */
	Term_activate(t);
}

/*
 * Initialization function
 */
errr init_nds(void)
{
	/* Initialize globals */

	/* Initialize "term_data" structures */

	int i;
	bool none = true;

	term_data *td;

	/* Main window */
	td = &data[0];
	memset(td, 0, sizeof(term_data));

	/* Create windows (backwards!) */
	for (i = MAX_TERM_DATA - 1; i >= 0; i--) {
		/* Link */
		term_data_link(i);
		none = false;

		/* Set global pointer */
		angband_term[0] = Term;
	}

	if (none)
		return (1);

	/* Success */
	return (0);
}

/*
 * Init some stuff
 *
 * This function is used to keep the "path" variable off the stack.
 */
static void init_stuff(void)
{
	char path[1024];

	/* Prepare the path */
	strcpy(path, "/angband/lib/");

	/* Prepare the filepaths */
	init_file_paths(path, path, path);

	/* Hack */
	// strcpy(savefile, "/angband/lib/save/PLAYER");
}

void nds_log(const char *msg)
{
	static byte x = 2, y = 1;
	byte i = 0;
	for (i = 0; msg[i] != '\0'; i++) {
		draw_char(x, y, msg[i]);
		x++;
		if (msg[i] == '\n' || x > (250 / NDS_FONT_WIDTH) - 2) {
			x = 2;
			y++;
		}
	}
}

void nds_logf(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	int len = vsnprintf(NULL, 0, format, args);

	char buf[len];

	vsprintf(buf, format, args);
	nds_log(buf);

	va_end(args);
}

/*should be replaced with open and read from z-file.c */
bool nds_load_file(const char *name, u16b *dest, u32b len)
{
	FILE *f = fopen(name, "r");
	if (f == NULL)
		return false;
	u16b readbuf[1024];
	u32b i, l, wi = 0;
	if (len == 0)
		len = 0xffffffff; /* max possible len */
	for (i = 0; i < 1024; i++)
		readbuf[i] = 0;
	while ((l = fread(readbuf, 2, 1024, f)) > 0 && wi * 2 < len) {
		for (i = 0; i < (l) && wi * 2 < len; i++) { /* 0 to l/2 */
			dest[wi++] = readbuf[i];
		}
		for (i = 0; i < 1024; i++)
			readbuf[i] = 0;
	}
	fclose(f);
	return true;
}

bool nds_load_kbd()
{
#define NUM_FILES 3
	const char *files[] = {
	    "/angband/nds/kbd.bin",
	    "/angband/nds/kbd.pal",
	    "/angband/nds/kbd.map",
	};
	const u16b *dests[] = {
	    (u16b *)BG_TILE_RAM_SUB(0),
	    BG_PALETTE_SUB,
	    (u16 *)BG_MAP_RAM_SUB(8),
	};

	u16b i;
	for (i = 0; i < NUM_FILES; i++) {
		if (!nds_load_file(files[i], dests[i], 0)) {
			nds_logf("Error opening %s (errno=%d)\n", files[i], errno);
			return false;
		}
	}
#undef NUM_FILES

	return true;
}

void kbd_init()
{
	u16b i;
	for (i = 0; i < 16; i++) {
		BG_PALETTE_SUB[i + 16] = BG_PALETTE_SUB[i] ^ 0x7FFF;
	}
}

void nds_init_buttons()
{
	u16b i, j;
	for (i = 0; i < (NDS_NUM_MAPPABLE << NDS_NUM_MODIFIER); i++) {
		for (j = 0; j < NDS_CMD_LENGTH; j++) {
			nds_btn_cmds[i][j] = 0;
		}
	}
	if (access(NDS_BUTTON_FILE, 0444) == -1) {
		/* Set defaults */
		for (i = 0; i < (NDS_NUM_MAPPABLE << NDS_NUM_MODIFIER); i++)
			nds_btn_cmds[i][0] = btn_defaults[i];

		return;
	}

	FILE *f = fopen(NDS_BUTTON_FILE, "r");
	fread(&nds_btn_cmds[0], NDS_CMD_LENGTH,
	      (NDS_NUM_MAPPABLE << NDS_NUM_MODIFIER), f);
	fclose(f);
}

void nds_raw_print(const char *str)
{
	static u16b x = 0, y = 32;
	while (*str) {
		draw_char(x, y, (u8)(*(str++)));
		x++;
		if (x > 78) {
			x = 0;
			y++;
			if (y > 34)
				y = 32;
		}
	}
	draw_char(x, y, 219);
	fflush(0);
}

/*
 * Display warning message (see "z-util.c")
 */
static void hook_plog(const char *str)
{
	/* Warning */
	if (str) {
		nds_raw_print(str);
	}
}

/*
 * Display error message and quit (see "z-util.c")
 */
static void hook_quit(const char *str)
{
	int i, j;

	/* Give a warning */
	if (str) {
		nds_log(str);
	}

	/* Bail */
	nds_exit(0);
}

void nds_exit(int code)
{
	u16b i;
	for (i = 0; i < 60; i++) {
		nds_updated = 0xFF;
		do_vblank(); /* wait 1 sec. */
	}
	systemShutDown();
}

/*
 * Main function
 *
 * This function must do a lot of stuff.
 */
int main(int argc, char *argv[])
{
	bool game_start = false;
	bool new_game = false;
	int i;

	/* Initialize the machine itself  */
	/*START NETHACK STUFF */

	powerOn(POWER_ALL_2D | POWER_SWAP_LCDS);
	videoSetMode(MODE_5_2D | DISPLAY_BG2_ACTIVE);
	videoSetModeSub(MODE_5_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG2_ACTIVE);
	vramSetBankA(VRAM_A_MAIN_BG_0x06000000); /* BG2, event buf, fonts */
	vramSetBankB(VRAM_B_MAIN_BG_0x06020000); /* for storage (tileset) */
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	vramSetBankD(VRAM_D_MAIN_BG_0x06040000); /* for storage (tileset) */
	vramSetBankE(VRAM_E_LCD);                /* for storage (WIN_TEXT) */
	vramSetBankF(VRAM_F_LCD);                /* for storage (WIN_TEXT) */
	REG_BG2CNT = BG_BMP16_256x256;
	REG_BG2PA = 1 << 8;
	REG_BG2PB = 0;
	REG_BG2PC = 0;
	REG_BG2PD = 1 << 8;
	REG_BG2Y = 0;
	REG_BG2X = 0;
	REG_BG0CNT_SUB = BG_TILE_BASE(0) | BG_MAP_BASE(8) | BG_PRIORITY(0) | BG_COLOR_16;
	REG_BG2CNT_SUB = BG_BMP16_256x256 | BG_BMP_BASE(2);
	REG_BG2PA_SUB = 1 << 8;
	REG_BG2PB_SUB = 0;
	REG_BG2PC_SUB = 0;
	REG_BG2PD_SUB = 1 << 8;
	REG_BG2Y_SUB = 0;
	REG_BG2X_SUB = 0;

	register int fd;

	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();

	if (!fatInitDefault()) {
		nds_log("\nError initializing FAT drivers.\n");
		nds_log("Make sure the game is patched with the correct DLDI.\n");
		nds_log(" (see https://www.chishm.com/DLDI/ for more info).\n");
		nds_log("\n\nUnable to access filesystem.\nCannot continue.\n");

		/* Lock up */
		while(1)
			swiWaitForVBlank();

		return 1;
	}

	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();
	swiWaitForVBlank();

	chdir("/angband");
	if (!nds_load_kbd()) {
		nds_log("\nError loading keyboard graphics.\nCannot continue.\n");

		/* Lock up */
		while(1)
			swiWaitForVBlank();

		return 1; /* die */
	}
	kbd_init();
	nds_init_buttons();

	/* Activate hooks */
	plog_aux = hook_plog;
	quit_aux = hook_quit;

	/* Initialize the windows */
	if (init_nds())
		quit("Oops!");

	/* XXX XXX XXX */
	ANGBAND_SYS = "nds";

	/* Initialize some stuff */
	init_stuff();

	/* About to start */
	game_start = true;

	while (game_start) {
		/* Initialize */
		init_display();
		init_angband();
		textui_init();

		/* Wait for response */
		pause_line(Term);

		/* Play the game */
		play_game(new_game);

		/* Free resources */
		textui_cleanup();
		cleanup_angband();
	}

	/* Quit */
	quit(NULL);

	/* Exit */
	return (0);
}

double sqrt(double x) {
	return f32tofloat(sqrtf32(floattof32(x)));
}
