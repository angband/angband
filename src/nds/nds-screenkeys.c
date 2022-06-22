#include "nds-screenkeys.h"

#include "../h-basic.h"
#include "nds-draw.h"
#include "nds-event.h"
#include "../z-file.h"
#include "../z-util.h"
#include "../z-virt.h"

#ifdef __3DS__
#include <3ds/types.h>
#include <3ds/services/hid.h>
#else
#include <nds.h>
#endif

#include "math.h"

#define NDS_SCRKEY_LABEL_LEN 16
#define NDS_SCRKEY_SEQ_LEN 20
#define NDS_SCRKEY_FILE "/angband/nds/screen-keys.txt"
#define NDS_SCRKEY_FILE_MAX_LINE 512

#define CLIP(val, min, max) MAX(MIN(val, max), min)

typedef struct {
	char label[NDS_SCRKEY_LABEL_LEN];
	uint16_t x, y;
	uint16_t w, h;
	char sequence[NDS_SCRKEY_SEQ_LEN];
} nds_scrkey_entry;

nds_scrkey_entry *nds_scrkeys = NULL;
int nds_scrkeys_num = 0;

/*
 * The format of the on-screen key definitions is line-based.
 * Empty lines and lines starting with a '#' will be ignored.
 *
 * A line has multiple fields, each separated by a ':'.
 * The fields in a line are:
 *  - label (up to NDS_SCRKEY_LABEL_LEN characters)
 *  - x and y offset (in pixels; from the top-left of the screen)
 *  - width and height (in pixels)
 *  - input sequence (up to NDS_SCRKEY_SEQ_LEN character inputs)
 *
 * Escaped characters in the input sequence are escaped using '\'
 * (most notably "\r", "\b", "\e", "\t" and "\\").
 * Arbitrary characters can be encoded in hexadecimal using "\x", in the format "\xa2".
 * Quotation marks and question marks are not escaped and can be used as-is.
 *
 * Encoding a null-byte will end the input sequence, even if more characters may follow.
 */
void nds_scrkey_add_file(ang_file *f) {
	char *line = mem_alloc(NDS_SCRKEY_FILE_MAX_LINE);

	while (file_getl(f, line, NDS_SCRKEY_FILE_MAX_LINE)) {
		if (line[0] == '\0' || line[0] == '#')
			continue;

		char *label = strtok(line, ":");
		char *x_str = strtok(NULL, ":");
		char *y_str = strtok(NULL, ":");
		char *w_str = strtok(NULL, ":");
		char *h_str = strtok(NULL, ":");
		char *sequence = strtok(NULL, "\n");

		nds_scrkey_entry entry = { 0 };

		strncpy(entry.label, label, NDS_SCRKEY_LABEL_LEN);

		entry.x = CLIP(strtoul(x_str, NULL, 0), 0, NDS_SCREEN_WIDTH);
		entry.y = CLIP(strtoul(y_str, NULL, 0), 0, NDS_SCREEN_HEIGHT);
		entry.w = CLIP(strtoul(w_str, NULL, 0), 0, NDS_SCREEN_WIDTH - entry.x);
		entry.h = CLIP(strtoul(h_str, NULL, 0), 0, NDS_SCREEN_HEIGHT - entry.y);

		strunescape(sequence);
		strncpy(entry.sequence, sequence, NDS_SCRKEY_SEQ_LEN);

		nds_scrkeys = mem_realloc(nds_scrkeys, (nds_scrkeys_num + 1) * sizeof(nds_scrkey_entry));
		nds_scrkeys[nds_scrkeys_num++] = entry;
	}

	mem_free(line);
}

void nds_scrkey_redraw_key(nds_scrkey_entry *key, bool initial, bool active)
{
	if (initial) {
		for (int y = 0; y < key->h; y++) {
			nds_draw_pixel(key->x,
			               NDS_SCREEN_HEIGHT + key->y + y,
			               NDS_WHITE_PIXEL);
			nds_draw_pixel(key->x + key->w,
			               NDS_SCREEN_HEIGHT + key->y + y,
			               NDS_WHITE_PIXEL);
		}

		for (int x = 0; x < key->w; x++) {
			nds_draw_pixel(key->x + x,
			               NDS_SCREEN_HEIGHT + key->y,
			               NDS_WHITE_PIXEL);
			nds_draw_pixel(key->x + x,
			               NDS_SCREEN_HEIGHT + key->y + key->h,
			               NDS_WHITE_PIXEL);
		}
	}

	int str_x = key->x + key->w / 2 - strnlen(key->label, NDS_SCRKEY_LABEL_LEN) * nds_font->width / 2;
	int str_y = NDS_SCREEN_HEIGHT + key->y + key->h / 2 - nds_font->height / 2;

	for (int i = 0; i < NDS_SCRKEY_LABEL_LEN && key->label[i]; i++) {
		nds_draw_char_px(str_x + (i * nds_font->width), str_y, key->label[i],
		                 active ? NDS_CURSOR_COLOR : NDS_WHITE_PIXEL, NDS_BLACK_PIXEL);
	}
}

void nds_scrkey_redraw(bool initial)
{
	/* Temporarily use the 5x8 font */
	const nds_font_handle *old_font = nds_font;
	nds_font = &nds_font_5x8;

	/* Redraw all keys */
	for (int i = 0; i < nds_scrkeys_num; i++) {
		nds_scrkey_redraw_key(&nds_scrkeys[i], initial, false);
	}

	nds_font = old_font;
}

void nds_scrkey_init()
{
	ang_file *f = file_open(NDS_SCRKEY_FILE, MODE_READ, FTYPE_TEXT);

	if (f) {
		nds_scrkey_add_file(f);
		file_close(f);
	}

	nds_scrkey_redraw(true);
}

void nds_scrkey_vblank()
{
	static bool need_redraw = false;

	if (need_redraw && (keysUp() & KEY_TOUCH)) {
		nds_scrkey_redraw(false);
		need_redraw = false;
	}

	if (!(keysDown() & KEY_TOUCH)) {
		return;
	}

	touchPosition touch;
	touchRead(&touch);

	for (int i = 0; i < nds_scrkeys_num; i++) {
		nds_scrkey_entry *entry = &nds_scrkeys[i];

		if (touch.px < entry->x || touch.py < entry->y ||
		    touch.px > entry->x + entry->w || touch.py > entry->y + entry->h) {
			continue;
		}

		/* Redraw key as "pressed" */
		const nds_font_handle *old_font = nds_font;
		nds_font = &nds_font_5x8;
		nds_scrkey_redraw_key(entry, false, true);
		nds_font = old_font;

		need_redraw = true;

		for (int si = 0; si < NDS_SCRKEY_SEQ_LEN && entry->sequence[si]; si++) {
			nds_event_put_key(entry->sequence[si], 0);
		}

		break;
	}
}
