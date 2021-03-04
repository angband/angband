#include "nds-buttons.h"

#include "../h-basic.h"
#include "nds-event.h"

#ifdef _3DS

#include <3ds.h>

void nds_btn_init()
{
}


void nds_btn_vblank()
{
	hidScanInput();

	u32 kd = hidKeysDown();

	/* Basic input mapping */
	if (kd & KEY_DRIGHT)
		nds_event_put_key('6');

	if (kd & KEY_DLEFT)
		nds_event_put_key('4');

	if (kd & KEY_DUP)
		nds_event_put_key('8');

	if (kd & KEY_DDOWN)
		nds_event_put_key('2');

	if (kd & KEY_X)
		nds_event_put_key('\e');

	if (kd & KEY_Y)
		nds_event_put_key('y');

	if (kd & KEY_A)
		nds_event_put_key('\r');
}

#endif