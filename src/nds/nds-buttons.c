#include "nds-buttons.h"

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

#define NDS_BTN_SEQ_LEN 20
#define NDS_BTN_FILE "/angband/nds/button-mappings.txt"
#define NDS_BTN_FILE_MAX_LINE 512

#define NDS_CPAD_MAX 154
#define NDS_CPAD_DEADZONE 20
#define NDS_CPAD_MIN_COOLDOWN 5

typedef struct {
	char input;
	float min;
	float max;
} nds_btn_cpad_zone;

typedef struct {
	char input[NDS_BTN_SEQ_LEN];
	u32 keys;
} nds_btn_map_entry;

const nds_btn_cpad_zone nds_btn_cpad_map[] = {
	{'6',	-22.5,	22.5},		/* right */
	{'9',	22.5,	67.5},		/* up right */
	{'8',	67.5,	122.5},		/* up */
	{'7',	122.5,	167.5}, 	/* up left */
	{'4',	167.5,	180},		/* left */
	{'4',	-180,	-167.5},	/* left */
	{'1',	-167.5,	-122.5},	/* down left */
	{'2',	-122.5,	-67.5},		/* down */
	{'3',	-67.5,	-22.5},		/* down right */
	{0, 0, 0},
};

#ifndef __3DS__
#define KEY_DUP KEY_UP
#define KEY_DDOWN KEY_DOWN
#define KEY_DLEFT KEY_LEFT
#define KEY_DRIGHT KEY_RIGHT

#define KEY_ZL 0
#define KEY_ZR 0

#define hidKeysDown keysDown
#define hidKeysHeld keysHeld
#endif

#define NDS_BTN_KEYS (KEY_DUP | KEY_DDOWN | KEY_DLEFT | KEY_DRIGHT | \
                      KEY_A | KEY_B | KEY_Y | KEY_X | KEY_START | KEY_SELECT)
#define NDS_BTN_MODS (KEY_L | KEY_R | KEY_ZL | KEY_ZR)
#define NDS_BTN_ALL (NDS_BTN_KEYS | NDS_BTN_MODS)

const nds_btn_map_entry nds_btn_default_map[] = {
	{"9",	KEY_DUP | KEY_DRIGHT},
	{"7",	KEY_DUP | KEY_DLEFT},
	{"1",	KEY_DDOWN | KEY_DLEFT},
	{"3",	KEY_DDOWN | KEY_DRIGHT},
	{"6",	KEY_DRIGHT},
	{"8",	KEY_DUP},
	{"4",	KEY_DLEFT},
	{"2",	KEY_DDOWN},
	{"\x1B",	KEY_B},
	{"y",	KEY_Y},
	{"\r",	KEY_A},
};

nds_btn_map_entry *nds_btn_map = NULL;
int nds_btn_map_num = 0;

void nds_btn_add_mappings(const nds_btn_map_entry *new_entries, int num) {
	nds_btn_map = mem_realloc(nds_btn_map, (nds_btn_map_num + num) * sizeof(nds_btn_map_entry));
	memcpy(&nds_btn_map[nds_btn_map_num], new_entries, num * sizeof(nds_btn_map_entry));
	nds_btn_map_num += num;
}

/*
 * The format of the button definitions is line-based.
 *
 * The first part of the line lists a set of keys, each delimited by a '+':
 *   "A", "B", "Y", "X", "Up", "Down", "Left", "Right", "Start", "Select",
 *   "L", "R", "ZL" and "ZR".
 * L, R, ZL and ZR are modifier keys and do not trigger an action on their own.
 *
 * The second part of the line is delimited by a ':', followed by a sequence of up to
 * NDS_BTN_SEQ_LEN character inputs that should be triggered.
 * Escaped characters are escaped using '\' (most notably "\r", "\b", "\e", "\t" and "\\").
 * Arbitrary characters can be encoded in hexadecimal using "\x", in the format "\xa2".
 * Quotation marks and question marks are not escaped and can be used as-is.
 *
 * Encoding a null-byte will end the input sequence, even if more characters may follow.
 *
 * User-defined button mappings may override any of the default mappings seen
 * in `nds_btn_default_map`. Earlier definitions will take precedence over later ones.
 *
 * Empty lines and lines starting with a '#' will be ignored.
 */
void nds_btn_add_mappings_from_file(ang_file *f) {
	char *line = mem_alloc(NDS_BTN_FILE_MAX_LINE);

	while (file_getl(f, line, NDS_BTN_FILE_MAX_LINE)) {
		if (line[0] == '\0' || line[0] == '#')
			continue;

		char *buttons = strtok(line, ":");
		char *sequence = strtok(NULL, "\n");

		nds_btn_map_entry entry = { 0 };

		char *button = strtok(buttons, "+");

		while (button) {
			if (streq(button, "A")) {
				entry.keys |= KEY_A;
			} else if (streq(button, "B")) {
				entry.keys |= KEY_B;
			} else if (streq(button, "Y")) {
				entry.keys |= KEY_Y;
			} else if (streq(button, "X")) {
				entry.keys |= KEY_X;
			} else if (streq(button, "Up")) {
				entry.keys |= KEY_DUP;
			} else if (streq(button, "Down")) {
				entry.keys |= KEY_DDOWN;
			} else if (streq(button, "Left")) {
				entry.keys |= KEY_DLEFT;
			} else if (streq(button, "Right")) {
				entry.keys |= KEY_DRIGHT;
			} else if (streq(button, "Start")) {
				entry.keys |= KEY_START;
			} else if (streq(button, "Select")) {
				entry.keys |= KEY_SELECT;
			} else if (streq(button, "L")) {
				entry.keys |= KEY_L;
			} else if (streq(button, "R")) {
				entry.keys |= KEY_R;
#ifdef __3DS__
			} else if (streq(button, "ZL")) {
				entry.keys |= KEY_ZL;
			} else if (streq(button, "ZR")) {
				entry.keys |= KEY_ZR;
#endif
			} else {
				nds_logf("Unknown button: '%s'\n", button);
			}

			button = strtok(NULL, "+");
		}

		strunescape(sequence);

		strncpy(entry.input, sequence, NDS_BTN_SEQ_LEN);

		nds_btn_add_mappings(&entry, 1);
	}

	mem_free(line);
}

void nds_btn_init()
{
	ang_file *f = file_open(NDS_BTN_FILE, MODE_READ, FTYPE_TEXT);

	if (f) {
		nds_btn_add_mappings_from_file(f);
		file_close(f);
	}

	nds_btn_add_mappings(nds_btn_default_map, N_ELEMENTS(nds_btn_default_map));
}

void nds_btn_check_cpad()
{
#ifdef __3DS__
	static uint8_t input_cooldown = 0;

	/* Skip if cooldown isn't expired yet */
	if (input_cooldown > 0) {
		input_cooldown--;
		return;
	}

	circlePosition pos;
	hidCircleRead(&pos);

	/* max. 154 */
	float cpad_rad = sqrt(pow(pos.dx, 2) + pow(pos.dy, 2));

	/* Input in deadzone? */
	if (cpad_rad < NDS_CPAD_DEADZONE)
		return;

	/* 0 = right, 90 = up, 180/-180 = left, -90 = down */
	float cpad_ang = atan2(pos.dy, pos.dx) * (180 / M_PI);

	/* Calculate the new cooldown */
	input_cooldown = (NDS_CPAD_MAX - cpad_rad) / 10;

	if (input_cooldown < NDS_CPAD_MIN_COOLDOWN)
		input_cooldown = NDS_CPAD_MIN_COOLDOWN;

	/* Trigger inputs */
	for (int i = 0; nds_btn_cpad_map[i].input; i++) {
		if (cpad_ang >= nds_btn_cpad_map[i].min &&
		    cpad_ang < nds_btn_cpad_map[i].max)
			nds_event_put_key(nds_btn_cpad_map[i].input, 0);
	}
#endif
}

void nds_btn_vblank()
{
	nds_btn_check_cpad();

	u32 kd = hidKeysDown();
	u32 kh = hidKeysHeld();

	/* Check all mapped inputs */
	for (int i = 0; i < nds_btn_map_num; i++) {
		u32 keys = nds_btn_map[i].keys;

		/* Check if all the buttons are held */
		if ((kh & NDS_BTN_ALL) != keys)
			continue;

		/*
		 * Check if at least one of the buttons has
		 * been pressed this frame
		 */
		if (!(kd & NDS_BTN_KEYS & keys))
			continue;

		for (int si = 0; si < NDS_BTN_SEQ_LEN && nds_btn_map[i].input[si]; si++) {
			nds_event_put_key(nds_btn_map[i].input[si], 0);
		}
		break;
	}
}
