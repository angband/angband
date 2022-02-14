#include "nds-keyboard.h"

#ifdef __3DS__
#include <3ds.h>
#else
#include <nds.h>
#endif

#include "nds-draw.h"
#include "nds-event.h"

#ifdef __3DS__
#define KBD_MARGIN	8
#define KBD_UNIT	10
#else
#define KBD_MARGIN	8
#define KBD_UNIT	8
#endif

#define KBD_KEY_HEIGHT	(KBD_UNIT*2)
#define KBD_PADDING	(KBD_UNIT/2)

#define KBD_MOD		(1 << 7)
#define KBD_CAPS	(KBD_MOD | (1 << 0))
#define KBD_SHIFT	(KBD_MOD | (1 << 1))
#define KBD_CTRL	(KBD_MOD | (1 << 2))
#define KBD_ALT		(KBD_MOD | (1 << 3))

/*
 * The width of a key is multiplied by KBD_UNIT.
 * If alt is 0, main stays as-is when shift is enabled.
 */
typedef struct {
	int width;
	char main;
	char alt;
} nds_kbd_key;

typedef struct {
	int length;
	const nds_kbd_key *keys;
} nds_kbd_row;

const nds_kbd_key nds_kbd_row0[] = {
	{2, '`', '~'},
	{2, '1', '!'},
	{2, '2', '@'},
	{2, '3', '#'},
	{2, '4', '$'},
	{2, '5', '%'},
	{2, '6', '^'},
	{2, '7', '&'},
	{2, '8', '*'},
	{2, '9', '('},
	{2, '0', ')'},
	{2, '-', '_'},
	{2, '=', '+'},
	{4, '\b', 0},
};

const nds_kbd_key nds_kbd_row1[] = {
	{3, '\t', 0},
	{2, 'q', 'Q'},
	{2, 'w', 'W'},
	{2, 'e', 'E'},
	{2, 'r', 'R'},
	{2, 't', 'T'},
	{2, 'y', 'Y'},
	{2, 'u', 'U'},
	{2, 'i', 'I'},
	{2, 'o', 'O'},
	{2, 'p', 'P'},
	{2, '[', '{'},
	{2, ']', '}'},
	{3, '\\', '|'},
};

const nds_kbd_key nds_kbd_row2[] = {
	{4, KBD_CAPS, 0},
	{2, 'a', 'A'},
	{2, 's', 'S'},
	{2, 'd', 'D'},
	{2, 'f', 'F'},
	{2, 'g', 'G'},
	{2, 'h', 'H'},
	{2, 'j', 'J'},
	{2, 'k', 'K'},
	{2, 'l', 'L'},
	{2, ';', ':'},
	{2, '\'', '"'},
	{4, '\n', 0},
};

const nds_kbd_key nds_kbd_row3[] = {
	{5, KBD_SHIFT, 0},
	{2, 'z', 'Z'},
	{2, 'x', 'X'},
	{2, 'c', 'C'},
	{2, 'v', 'V'},
	{2, 'b', 'B'},
	{2, 'n', 'N'},
	{2, 'm', 'M'},
	{2, ',', '<'},
	{2, '.', '>'},
	{2, '/', '?'},
	{5, KBD_SHIFT, 0},
};

const nds_kbd_key nds_kbd_row4[] = {
	{4, KBD_CTRL, 0},
	{3, KBD_ALT, 0},
	{16, ' ', 0},
	{3, KBD_ALT, 0},
	{4, KBD_CTRL, 0},
};

const nds_kbd_row nds_kbd_map[] = {
	{N_ELEMENTS(nds_kbd_row0), nds_kbd_row0},
	{N_ELEMENTS(nds_kbd_row1), nds_kbd_row1},
	{N_ELEMENTS(nds_kbd_row2), nds_kbd_row2},
	{N_ELEMENTS(nds_kbd_row3), nds_kbd_row3},
	{N_ELEMENTS(nds_kbd_row4), nds_kbd_row4},
};

static char nds_kbd_active_mods;

/*
 * Translates characters into their string representation.
 * Returns NULL if no special handling is required.
 */
char *nds_kbd_char_string(char c)
{
	/* Try all the special characters */
	switch (c) {
	case '\b':
		return "Bksp";
	case '\t':
		return "Tab";
	case '\n':
		return "Enter";
	case KBD_CAPS:
		return "Caps";
	case KBD_SHIFT:
		return "Shift";
	case KBD_CTRL:
		return "Ctrl";
	case KBD_ALT:
		return "Alt";
	}

	/* Fall back to just printing the character */
	return NULL;
}

bool nds_kbd_modifier_active(char mod) {
	/* Key can't be active if it isn't a modifier */
	if (!(mod & KBD_MOD))
		return false;

	/* Discard the KBD_MOD identifier */
	mod &= ~(KBD_MOD);

	return (nds_kbd_active_mods & mod) == mod;
}

void nds_kbd_redraw_key(int r, int k, bool initial, bool active)
{
	/* Fetch the matching row and key */
	nds_kbd_row row = nds_kbd_map[r];
	nds_kbd_key key = row.keys[k];

	/* Calculate the row offset */
	int row_offset = KBD_MARGIN + r * KBD_KEY_HEIGHT;

	/* Calculate the key offset */
	int key_offset = KBD_MARGIN;
	for (int i = 0; i < k; i++) {
		key_offset += row.keys[i].width * KBD_UNIT;
	}

	int key_width = key.width * KBD_UNIT;

	if (initial) {
		/* Draw the outline (left and right) */
		for (int y = 0; y < KBD_KEY_HEIGHT; y++) {
			nds_draw_pixel(key_offset,
			               NDS_SCREEN_HEIGHT + row_offset + y,
			               NDS_WHITE_PIXEL);
			nds_draw_pixel(key_offset + key_width,
			               NDS_SCREEN_HEIGHT + row_offset + y,
			               NDS_WHITE_PIXEL);
		}

		/* Draw the outline (top and bottom) */
		for (int x = 0; x < key_width; x++) {
			nds_draw_pixel(key_offset + x,
			               NDS_SCREEN_HEIGHT + row_offset,
			               NDS_WHITE_PIXEL);
			nds_draw_pixel(key_offset + x,
			               NDS_SCREEN_HEIGHT + row_offset + KBD_KEY_HEIGHT,
			               NDS_WHITE_PIXEL);
		}
	}

	/* The key is active if it's a modifier and the modifier is active */
	active |= nds_kbd_modifier_active(key.main);

	/* We are shifted if SHIFT is on... */
	bool shifted = nds_kbd_modifier_active(KBD_SHIFT);

	/* ... or CAPS is on and we are between a and z */
	shifted |= nds_kbd_modifier_active(KBD_CAPS) &&
	           key.main >= 'a' && key.main <= 'z';

	/* Don't shift if there is nothing to shift to */
	shifted &= (key.alt != 0);

	/* If we are shifted use the alternative key */
	char c = shifted ? key.alt : key.main;
	char *s = nds_kbd_char_string(c);

	/* Final printing coordinates */
	int str_x = key_offset + KBD_PADDING;
	int str_y = NDS_SCREEN_HEIGHT + row_offset +
	            (KBD_KEY_HEIGHT - nds_font->height - KBD_PADDING);

	/* If no special handling is required, just print the char */
	if (s == NULL) {
		nds_draw_char_px(str_x, str_y, c,
		                 active ? NDS_CURSOR_COLOR : NDS_WHITE_PIXEL, NDS_BLACK_PIXEL);
		return;
	}

	/* Print the text */
	for (int i = 0; i < strlen(s); i++) {
		nds_draw_char_px(str_x + (i * nds_font->width), str_y, s[i],
		                 active ? NDS_CURSOR_COLOR : NDS_WHITE_PIXEL, NDS_BLACK_PIXEL);
	}
}

void nds_kbd_redraw(bool initial)
{
	/* Temporarily use the 5x8 font */
	const nds_font_handle *old_font = nds_font;
	nds_font = &nds_font_5x8;

	/* Redraw all keys */
	for (int r = 0; r < N_ELEMENTS(nds_kbd_map); r++) {
		for (int k = 0; k < nds_kbd_map[r].length; k++) {
			nds_kbd_redraw_key(r, k, initial, false);
		}
	}

	nds_font = old_font;
}

bool nds_kbd_init()
{
	nds_kbd_active_mods = 0;

	/* Fill background with black */
	for (int x = 0; x < NDS_SCREEN_WIDTH; x++) {
		for (int y = NDS_SCREEN_HEIGHT; y < NDS_SCREEN_HEIGHT * 2; y++) {
			nds_draw_pixel(x, y, NDS_BLACK_PIXEL);
		}
	}

	nds_kbd_redraw(true);

	return true;
}

void nds_kbd_vblank()
{
	static bool need_redraw = false;

	if (need_redraw && (keysUp() & KEY_TOUCH)) {
		nds_kbd_redraw(false);
		need_redraw = false;
	}

	/* Nothing to do if no touch input */
	if (!(keysDown() & KEY_TOUCH))
		return;

	/* Read current touch input */
	touchPosition touch;
	touchRead(&touch);

	/* To the top or the left of the keyboard? */
	if (touch.px < KBD_MARGIN || touch.py < KBD_MARGIN)
		return;

	int r = (touch.py - KBD_MARGIN) / KBD_KEY_HEIGHT;
	int x = touch.px - KBD_MARGIN;
	int k = -1;

	/* Below the keyboard? */
	if (r >= N_ELEMENTS(nds_kbd_map))
		return;

	nds_kbd_row row = nds_kbd_map[r];

	for (int i = 0; i < row.length; i++) {
		x -= row.keys[i].width * KBD_UNIT;

		/* Found the correct key */
		if (x < 0) {
			k = i;
			break;
		}
	}

	/* No key found (i.e. to the right of the keyboard)? */
	if (k == -1) {
		return;
	}

	/* We will change something, so update next frame */
	need_redraw = true;

	nds_kbd_key key = row.keys[k];

	/* Redraw key as "pressed" */
	const nds_font_handle *old_font = nds_font;
	nds_font = &nds_font_5x8;
	nds_kbd_redraw_key(r, k, false, true);
	nds_font = old_font;

	/* If it's a modifier, toggle it and return */
	if (key.main & KBD_MOD) {
		nds_kbd_active_mods ^= key.main;
		return;
	}

	/* Do we have shift enabled? */
	bool shift = nds_kbd_modifier_active(KBD_SHIFT);

	/* Do we have caps-lock enabled (and are within range)? */
	shift |= nds_kbd_modifier_active(KBD_CAPS) &&
	         key.main >= 'a' && key.main <= 'z';

	/* Do we have something to shift to? */
	shift &= (key.alt != 0);

	char c = shift ? key.alt : key.main;

	uint8_t mods = 0;

	/* SHIFT only gets passed as a modifier if we haven't applied it to the keycode */
	if (nds_kbd_modifier_active(KBD_SHIFT) && !shift)
		mods |= KC_MOD_SHIFT;

	if (nds_kbd_modifier_active(KBD_CTRL)) {
		/* Automatically make 'a' - 'z' uppercase for convenience */
		if (c >= 'a' && c <= 'z')
			c -= 0x20;

		if (ENCODE_KTRL(c))
			c = KTRL(c);
		else
			mods |= KC_MOD_CONTROL;
	}

	if (nds_kbd_modifier_active(KBD_ALT))
		mods |= KC_MOD_ALT;

	nds_event_put_key(c, mods);

	/* Remove all modifiers except for caps */
	nds_kbd_active_mods &= KBD_CAPS;
}
