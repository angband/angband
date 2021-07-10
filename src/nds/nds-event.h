#ifndef _NDS_EVENT_H
#define _NDS_EVENT_H

#include "../h-basic.h"

#define MAX_EBUF	512
#define MEVENT_FLAG	(1<<15)
#define EVENT_SET	(1<<14)

#define EVENT_X(e)	((uint8_t)((e) & 0x7F))
#define EVENT_Y(e)	((uint8_t)(((e) & 0xF8) >> 7))
#define EVENT_C(e)	((uint8_t)((e) & 0xFF))
#define IS_MEVENT(e)	((e) & MEVENT_FLAG)

bool nds_event_init();
bool nds_event_ready();
u16b nds_event_get();
void nds_event_put_key(byte c);
void nds_event_put_mouse(byte x, byte y);

#endif