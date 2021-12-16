#include "nds-event.h"

#ifndef _3DS
#include <nds.h>
#endif

#define MAX_EBUF	64

/* Event queue ring buffer */
nds_event *ebuf;

/* Read and write pointers, that always point to the next element */
uint16_t ebuf_read = 0, ebuf_write = 0;

nds_event empty_event = { 0 };

bool nds_event_init() {
	ebuf = (nds_event *) malloc(sizeof(nds_event) * MAX_EBUF);

	if (!ebuf) {
		return false;
	}

	/* Make sure that we don't read garbage events */
	memset(ebuf, 0, sizeof(nds_event) * MAX_EBUF);

	return true;
}

bool nds_event_ready()
{
	return (ebuf[ebuf_read].type != NDS_EVENT_INVALID);
}

nds_event nds_event_get()
{
	if (!nds_event_ready())
		return empty_event;
	nds_event event = ebuf[ebuf_read];
	ebuf[ebuf_read].type = NDS_EVENT_INVALID;
	if (++ebuf_read >= MAX_EBUF)
		ebuf_read = 0;
	return event;
}

void nds_event_put_key(keycode_t key, uint8_t mods)
{
	ebuf[ebuf_write].type = NDS_EVENT_KEYBOARD;
	ebuf[ebuf_write].keyboard.key = key;
	ebuf[ebuf_write].keyboard.mods = mods;
	if (++ebuf_write >= MAX_EBUF)
		ebuf_write = 0;
}

void nds_event_put_mouse(uint8_t x, uint8_t y)
{
	ebuf[ebuf_write].type = NDS_EVENT_MOUSE;
	ebuf[ebuf_write].mouse.x = x;
	ebuf[ebuf_write].mouse.y = y;
	if (++ebuf_write >= MAX_EBUF)
		ebuf_write = 0;
}
