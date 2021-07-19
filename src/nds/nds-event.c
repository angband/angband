#include "nds-event.h"

#ifndef _3DS
#include <nds.h>
#endif

/* Mainly imposed by NDS, but 85 events should be more than enough. */
#define MAX_EBUF	(1024 / sizeof(nds_event))

/* Event queue ring buffer and its read/write pointers */
#ifdef _3DS
nds_event *ebuf;
#else
nds_event *ebuf = (nds_event *)(&BG_GFX[256 * 192]);
#endif
u16b ebuf_read = 0, ebuf_write = 0;
nds_event empty_event = { 0 };

bool nds_event_init() {
#ifdef _3DS
	ebuf = (nds_event *) malloc(sizeof(nds_event) * MAX_EBUF);

	if (!ebuf) {
		return false;
	}
#endif

	return true;
}

bool nds_event_ready()
{
	return ((ebuf_read < ebuf_write) && ebuf[ebuf_read].type != NDS_EVENT_INVALID);
}

nds_event nds_event_get()
{
	if (!nds_event_ready())
		return empty_event;
	nds_event event = ebuf[ebuf_read];
	ebuf[ebuf_read].type = NDS_EVENT_INVALID;
	ebuf_read++;
	if (ebuf_read > ebuf_write) {
		ebuf_write++;
		if (ebuf_write >= MAX_EBUF)
			ebuf_write = 0;
	}
	if (ebuf_read >= MAX_EBUF)
		ebuf_read = 0;
	return event;
}

void nds_event_put_key(keycode_t key, byte mods)
{
	ebuf[ebuf_write].type = NDS_EVENT_KEYBOARD;
	ebuf[ebuf_write].keyboard.key = key;
	ebuf[ebuf_write].keyboard.mods = mods;
	if (++ebuf_write >= MAX_EBUF)
		ebuf_write = 0;
}

void nds_event_put_mouse(byte x, byte y)
{
	ebuf[ebuf_write].type = NDS_EVENT_MOUSE;
	ebuf[ebuf_write].mouse.x = x;
	ebuf[ebuf_write].mouse.y = y;
	if (++ebuf_write >= MAX_EBUF)
		ebuf_write = 0;
}