#include "nds-event.h"

#include <nds.h>

/* Event queue ring buffer and its read/write pointers */
u16b *ebuf = (u16b *)(&BG_GFX[256 * 192]);
u16b ebuf_read = 0, ebuf_write = 0;

bool nds_event_ready()
{
	/* read < write should never happen without EVENT_SET, but */
	/* just in case... */
	return ((ebuf[ebuf_read] & EVENT_SET) || (ebuf_read < ebuf_write));
}

u16b nds_event_get()
{
	if (!nds_event_ready())
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

void nds_event_put_key(byte c)
{
	ebuf[ebuf_write++] = EVENT_SET | (u16b)c;
	if (ebuf_write >= MAX_EBUF)
		ebuf_write = 0;
}

void nds_event_put_mouse(byte x, byte y)
{
	ebuf[ebuf_write++] = EVENT_SET | MEVENT_FLAG | (u16b)x | (((u16b)y) << 7);
	if (ebuf_write >= MAX_EBUF)
		ebuf_write = 0;
}