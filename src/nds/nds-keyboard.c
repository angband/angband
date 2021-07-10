#include "nds-keyboard.h"

#include "../z-file.h"
#include "nds-draw.h"
#include "nds-event.h"

#ifndef _3DS

typedef struct {
	u16b width;
	u16b code;
} nds_kbd_key;

#define K_MODIFIER	0x100
#define K_CAPS		0x101
#define K_SHIFT		0x102
#define K_CTRL		0x103
#define K_ALT		0x104
#define K_F(n)		(0x200 + n)
#define K_SHIFTED_MOVE	0x0200

#include <nds.h>

const nds_kbd_key row0[] = {{16, (u16b)'`'}, {16, (u16b)'1'},  {16, (u16b)'2'},
                            {16, (u16b)'3'}, {16, (u16b)'4'},  {16, (u16b)'5'},
                            {16, (u16b)'6'}, {16, (u16b)'7'},  {16, (u16b)'8'},
                            {16, (u16b)'9'}, {16, (u16b)'0'},  {16, (u16b)'-'},
                            {16, (u16b)'='}, {32, (u16b)'\b'}, {0, 0}};
const nds_kbd_key row1[] = {{24, (u16b)'\t'}, {16, (u16b)'q'},  {16, (u16b)'w'},
                            {16, (u16b)'e'},  {16, (u16b)'r'},  {16, (u16b)'t'},
                            {16, (u16b)'y'},  {16, (u16b)'u'},  {16, (u16b)'i'},
                            {16, (u16b)'o'},  {16, (u16b)'p'},  {16, (u16b)'['},
                            {16, (u16b)']'},  {24, (u16b)'\\'}, {0, 0}};
const nds_kbd_key row2[] = {{32, K_CAPS},    {16, (u16b)'a'}, {16, (u16b)'s'},
                            {16, (u16b)'d'},  {16, (u16b)'f'}, {16, (u16b)'g'},
                            {16, (u16b)'h'},  {16, (u16b)'j'}, {16, (u16b)'k'},
                            {16, (u16b)'l'},  {16, (u16b)';'}, {16, (u16b)'\''},
                            {32, (u16b)'\n'}, {0, 0}};
const nds_kbd_key row3[] = {{40, K_SHIFT},  {16, (u16b)'z'}, {16, (u16b)'x'},
                            {16, (u16b)'c'}, {16, (u16b)'v'}, {16, (u16b)'b'},
                            {16, (u16b)'n'}, {16, (u16b)'m'}, {16, (u16b)','},
                            {16, (u16b)'.'}, {16, (u16b)'/'}, {40, K_SHIFT},
                            {0, 0}};
const nds_kbd_key row4[] = {{32, K_CTRL}, {24, K_ALT},  {128, (u16b)' '},
                            {24, K_ALT},  {32, K_CTRL}, {0, 0}};
char shifts[] = "`~1!2@3#4$5%6^7&8*9(0)-_=+[{]}\\|;:\'\",<.>/?";
const nds_kbd_key *kbdrows[] = {row0, row1, row2, row3, row4};


static bool shift = false, ctrl = false, alt = false, caps = false;

u16b nds_kbd_mod_code(u16b ret)
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

void nds_kbd_set_color_from_pos(u16b r, u16b k, byte color)
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
			map[jj][(7 + r * 2) * 32 + ii + xx + 1] =
			    (map[jj][(7 + r * 2) * 32 + ii + xx + 1] &
			     0x0FFF) |
			    (color << 12);
			map[jj][(7 + r * 2 + 1) * 32 + ii + xx + 1] =
			    (map[jj][(7 + r * 2 + 1) * 32 + ii + xx + 1] &
			     0x0FFF) |
			    (color << 12);
		}
	}
}

void nds_kbd_set_color_from_code(u16b code, byte color)
{
	u16b r, k;
	for (r = 0; r < 5; r++) {
		for (k = 0; kbdrows[r][k].width != 0; k++) {
			if (nds_kbd_mod_code(kbdrows[r][k].code) == code) {
				nds_kbd_set_color_from_pos(r, k, color);
			}
			/* 
			 * do not break!! there may be >1 key with this code
			 * (modifier keys)
			 */
		}
	}
}

void nds_kbd_set_map()
{
	REG_BG0CNT_SUB = BG_TILE_BASE(0) |
	                 BG_MAP_BASE(8 + (caps | (shift << 1))) |
	                 BG_PRIORITY(0) | BG_COLOR_16;
}

u16b nds_kbd_xy2key(byte x, byte y)
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
	return nds_kbd_mod_code(ret);
}

void nds_kbd_dotoggle(bool *flag, int how)
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
void nds_kbd_togglemod(int which, int how)
{
	/* boolean old_shift = shift, old_ctrl = ctrl, old_alt = alt, old_caps = caps; */
	switch (which) {
	case K_CTRL:
		nds_kbd_dotoggle(&ctrl, how);
		break;
	case K_SHIFT:
		nds_kbd_dotoggle(&shift, how);
		break;
	case K_ALT:
		nds_kbd_dotoggle(&alt, how);
		break;
	case K_CAPS:
		nds_kbd_dotoggle(&caps, how);
		break;
	case K_MODIFIER:
		nds_kbd_dotoggle(&ctrl, how);
		nds_kbd_dotoggle(&shift, how);
		nds_kbd_dotoggle(&alt, how);
		/*
		 * NOT caps!!  This is called to un-set shift, ctrl, and alt after
		 * a key is pressed.  Unsetting caps here would cause it to be the
		 * same as shift.
		 */
		break;
	}

	/* if (old_shift != shift) */
	nds_kbd_set_color_from_code(K_SHIFT, shift);

	/* if (old_ctrl != ctrl) */
	nds_kbd_set_color_from_code(K_CTRL, ctrl);

	/* if (old_alt != alt) */
	nds_kbd_set_color_from_code(K_ALT, alt);

	/* if (old_caps != caps) */
	nds_kbd_set_color_from_code(K_CAPS, caps);

	nds_kbd_set_map();
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
void nds_kbd_vblank()
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
			nds_kbd_set_color_from_code(last_code, 0);
			/* and also clear all modifiers (except caps)    */
			nds_kbd_togglemod(K_MODIFIER, 0);
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
		keycode = nds_kbd_xy2key(the_x, the_y);

		/* if it's not a modifier, highlight it */
		if (keycode && !(keycode & K_MODIFIER))
			nds_kbd_set_color_from_code(keycode, 1);
		/* set last_code so it can be un-highlighted later */
		last_code = keycode;

		/*/* check for special keystrokes: alt-b, f5, f6 */
		if (process_special_keystrokes) {
			/* alt-b: assign button macro */
			if (keycode == ('b' | 0x80)) {
				/* clear hiliting */
				nds_kbd_set_color_from_code(keycode, 0);
				nds_kbd_togglemod(K_MODIFIER, 0);
				// nds_assign_button();
				keycode = last_code =
				    0; /* don't let nethack process it */
			}

			if (keycode & K_F(0)) { /* its an f-key */
				nds_kbd_togglemod(K_MODIFIER, 0);
			}
		}

		/* if it's a modifier, toggle it */
		if (keycode & K_MODIFIER)
			nds_kbd_togglemod(keycode, -1);
		else if ((keycode & 0x7F) != 0) { /* it's an actual keystroke */
			nds_event_put_key(keycode & 0xFF);
		}
	}

	return;
}

bool nds_kbd_init()
{
	const char *files[] = {
	    "/angband/nds/kbd.bin",
	    "/angband/nds/kbd.pal",
	    "/angband/nds/kbd.map",
	};
	char *dests[] = {
	    (char *) BG_TILE_RAM_SUB(0),
	    (char *) BG_PALETTE_SUB,
	    (char *) BG_MAP_RAM_SUB(8),
	};

	for (int i = 0; i < N_ELEMENTS(files); i++) {
		ang_file *handle = file_open(files[i], MODE_READ, -1);

		if (!handle) {
			nds_logf("Error opening '%s'\n", files[i]);
			return false;
		}

		char *dest = dests[i];
		int read_bytes;

		while ((read_bytes = file_read(handle, dest, 1024))) {
			if (read_bytes == -1) {
				nds_logf("Error reading '%s'\n", files[i]);
				return false;
			}

			if (read_bytes != 1024) {
				/* We're done reading the file */
				break;
			}

			dest += read_bytes;
		}

		file_close(handle);
	}

    /* Create inverted copy of the colors */
	for (u16b i = 0; i < 16; i++) {
		BG_PALETTE_SUB[i + 16] = BG_PALETTE_SUB[i] ^ 0x7FFF;
	}

	return true;
}

#endif