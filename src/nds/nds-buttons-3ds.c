#include "nds-buttons.h"

#include "../h-basic.h"
#include "nds-event.h"

#ifdef _3DS

#include <3ds.h>

#include "math.h"

#define NDS_CPAD_MAX 154
#define NDS_CPAD_DEADZONE 20
#define NDS_CPAD_MIN_COOLDOWN 5

typedef struct {
	char input;
	float min;
	float max;
} nds_btn_cpad_zone;

typedef struct {
	char input;
	u32 keys;
} nds_btn_map;

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

/* Most specific combinations first! */
const nds_btn_map nds_btn_default_map[] = {
	{'9',	KEY_DUP | KEY_DRIGHT},
	{'7',	KEY_DUP | KEY_DLEFT},
	{'1',	KEY_DDOWN | KEY_DLEFT},
	{'3',	KEY_DDOWN | KEY_DRIGHT},
	{'6',	KEY_DRIGHT},
	{'8',	KEY_DUP},
	{'4',	KEY_DLEFT},
	{'2',	KEY_DDOWN},
	{'\e',	KEY_X},
	{'y',	KEY_Y},
	{'\r',	KEY_A},
	{0, 0},
};

void nds_btn_init()
{
}

void nds_btn_check_cpad()
{
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
			nds_event_put_key(nds_btn_cpad_map[i].input);
	}
}

void nds_btn_vblank()
{
	hidScanInput();

	nds_btn_check_cpad();

	u32 kd = hidKeysDown();
	u32 kh = hidKeysHeld();

	/* Check all mapped inputs */
	for (int i = 0; nds_btn_default_map[i].input; i++) {
		u32 keys = nds_btn_default_map[i].keys;

		/* Check if all the buttons are held */
		if ((kh & keys) != keys)
			continue;

		/*
		 * Check if at least one of the buttons has
		 * been pressed this frame
		 */
		if (!(kd & keys))
			continue;

		nds_event_put_key(nds_btn_default_map[i].input);
		break;
	}
}

#endif