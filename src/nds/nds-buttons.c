#include "nds-buttons.h"

#include "../h-basic.h"
#include "nds-event.h"

#ifndef _3DS

#include <nds.h>

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

void nds_btn_vblank() {
	/* Check hardware for new inputs */
	scanKeys();

	/* Retrieve currently held and newly pressed keys */
	u32b kd = keysDown();
	u32b kh = keysHeld();
	
	/* Map of right, left, up, down and their combinations */
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
					nds_event_put_key(k2d[i]);
		} else if (dirs_down == 2 && (kh & (KEY_R | KEY_L))) {
			for (i = 0; i < 4; i++)
				if (kh & (1 << (i + 4)))
					nds_event_put_key(k2d[i + 4]);
		}
	}

	/* Check for special button mappings */
	s16b btn = nds_buttons_to_btnid(kd, kh);
	if (btn == -1)
		return;
	byte *cmd = &nds_btn_cmds[btn][0];
	while (*cmd != 0) {
		nds_event_put_key(*(cmd++));
	}
}

void nds_btn_init()
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

#endif